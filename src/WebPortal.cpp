#include "WebPortal.h"

#include <ESPmDNS.h>
#include <M5Unified.h>

#include "EmbeddedAssets.generated.h"

namespace {
constexpr uint16_t kDnsPort = 53;
constexpr size_t kMaxStoredBackupBytes = 420 * 1024;
constexpr char kSaveBackupPath[] = "/pocketboy-saves.json";
constexpr char kSaveUploadPath[] = "/pocketboy-saves.upload";
constexpr char kSavePreviousPath[] = "/pocketboy-saves.previous";
constexpr size_t kMaxStoredGameBytes = 1800 * 1024;
constexpr char kGameUploadPath[] = "/.pocketboy-upload";

String sanitizeGameName(String name) {
    name.replace('\\', '/');
    const int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.substring(slash + 1);
    String lower = name;
    lower.toLowerCase();
    String extension;
    if (lower.endsWith(".gbc")) extension = ".gbc";
    else if (lower.endsWith(".gb")) extension = ".gb";
    else return "";

    String stem = name.substring(0, name.length() - extension.length());
    stem.trim();
    String safe;
    safe.reserve(64);
    for (size_t i = 0; i < stem.length() && safe.length() < 54; ++i) {
        const char c = stem[i];
        const bool alphaNumeric = (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
        if (alphaNumeric || c == ' ' || c == '-' || c == '_' || c == '(' || c == ')') {
            safe += c;
        } else {
            safe += '_';
        }
    }
    safe.trim();
    if (safe.isEmpty()) safe = "game";
    return safe + extension;
}

bool isValidGameName(const String &name) {
    return !name.isEmpty() && sanitizeGameName(name) == name;
}

String jsonEscape(const String &value) {
    String escaped;
    escaped.reserve(value.length() + 8);
    for (size_t i = 0; i < value.length(); ++i) {
        const char c = value[i];
        if (c == '\"' || c == '\\') {
            escaped += '\\';
            escaped += c;
        } else if (static_cast<uint8_t>(c) < 0x20) {
            escaped += '_';
        } else {
            escaped += c;
        }
    }
    return escaped;
}

const char kPage[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#111522">
<title>PocketBoy S3 Controller Lab</title>
<style>
:root{color-scheme:dark;--bg:#090b12;--card:#151a28;--line:#2d354b;--ink:#f4f6ff;--muted:#aeb8d3;--green:#66e3a4;--yellow:#ffd166;--red:#ff6b7a;--blue:#73a7ff}
*{box-sizing:border-box}body{margin:0;min-height:100vh;background:radial-gradient(circle at 50% 0,#26304a 0,#101522 38%,var(--bg) 78%);color:var(--ink);font:16px system-ui,-apple-system,sans-serif}
main{width:min(900px,100%);margin:auto;padding:18px}.hero{text-align:center;padding:10px 0 18px}.eyebrow{color:var(--green);font-size:12px;font-weight:900;letter-spacing:.16em;text-transform:uppercase}h1{margin:6px 0;font-size:clamp(32px,8vw,54px)}.sub{margin:0;color:var(--muted)}
.grid{display:grid;grid-template-columns:1.15fr .85fr;gap:14px}.card{border:1px solid var(--line);border-radius:20px;background:#151a28e8;padding:16px;box-shadow:0 16px 45px #0005}.card h2{margin:0 0 12px;font-size:20px}.status{display:grid;gap:8px}.row{display:flex;align-items:center;justify-content:space-between;gap:10px;padding:9px 11px;border-radius:11px;background:#0d111c}.value{color:var(--muted);text-align:right}.ok{color:var(--green)}.warn{color:var(--yellow)}.bad{color:var(--red)}
button,.launch{border:0;border-radius:13px;background:var(--blue);color:#07101f;padding:12px 15px;font:inherit;font-weight:900;cursor:pointer;text-decoration:none;display:inline-block}button:active,.launch:active{transform:translateY(1px)}#activate{width:100%;margin-top:12px}.launch{width:100%;margin-top:10px;text-align:center;background:var(--green)}.pad{position:relative;min-height:285px;margin-top:2px;border-radius:18px;background:linear-gradient(145deg,#262e43,#111522);overflow:hidden}.pad:before{content:'';position:absolute;inset:28px 12%;border-radius:44% 44% 36% 36%;background:#343d54;box-shadow:inset 0 -14px #1d2332}
.btn{position:absolute;display:grid;place-items:center;width:42px;height:42px;border:2px solid #ffffff22;border-radius:50%;background:#101522;color:#fff;font-weight:900;transition:.05s}.btn.on{background:var(--green);color:#07140d;box-shadow:0 0 20px #66e3a488}.a{right:42px;top:105px}.b{right:88px;top:145px}.x{right:88px;top:65px}.y{right:134px;top:105px}.start{width:54px;height:25px;border-radius:12px;left:calc(50% + 10px);top:124px;font-size:10px}.select{width:54px;height:25px;border-radius:12px;right:calc(50% + 10px);top:124px;font-size:10px}
.d{position:absolute;left:44px;top:87px;width:105px;height:105px}.d i{position:absolute;display:block;background:#0b0e16;border:2px solid #ffffff18}.du,.dd{left:35px;width:35px;height:38px}.du{top:0;border-radius:8px 8px 2px 2px}.dd{bottom:0;border-radius:2px 2px 8px 8px}.dl,.dr{top:35px;width:38px;height:35px}.dl{left:0;border-radius:8px 2px 2px 8px}.dr{right:0;border-radius:2px 8px 8px 2px}.d i.on{background:var(--green)}
.axes{position:absolute;left:28px;right:28px;bottom:17px;text-align:center;color:var(--muted);font:12px ui-monospace,monospace}.mapping{margin:0;padding-left:20px;color:var(--muted);line-height:1.65}.mapping strong{color:var(--ink)}.log{min-height:48px;margin-top:12px;padding:10px;border-radius:12px;background:#090c14;color:var(--muted);font:12px ui-monospace,monospace;white-space:pre-wrap}.foot{text-align:center;color:var(--muted);font-size:12px;margin:16px 0 4px}
@media(max-width:700px){main{padding:11px}.grid{grid-template-columns:1fr}.pad{min-height:270px}}
</style>
</head>
<body><main>
<header class="hero"><div class="eyebrow">M5StickS3 browser experiment</div><h1>PocketBoy S3</h1><p class="sub">Chrome controller compatibility lab</p></header>
<div class="grid">
  <section class="card"><h2>Controller</h2><div class="status">
    <div class="row"><span>Page security</span><strong id="secure" class="value">Checking…</strong></div>
    <div class="row"><span>Gamepad API</span><strong id="api" class="value">Checking…</strong></div>
    <div class="row"><span>Detected controller</span><strong id="name" class="value">None</strong></div>
    <div class="row"><span>Mapping</span><strong id="mapping" class="value">—</strong></div>
    <div class="row"><span>PocketBoy battery</span><strong id="battery" class="value">Checking…</strong></div>
  </div><button id="activate">Activate controller</button><a class="launch" href="/play">Open game player</a><div id="log" class="log">Pair the controller in the phone's Bluetooth settings, then press a controller button.</div></section>
  <section class="card"><h2>Live input</h2><div class="pad">
    <div class="d"><i id="up" class="du"></i><i id="down" class="dd"></i><i id="left" class="dl"></i><i id="right" class="dr"></i></div>
    <span id="ba" class="btn a">A</span><span id="bb" class="btn b">B</span><span id="bx" class="btn x">X</span><span id="by" class="btn y">Y</span><span id="start" class="btn start">START</span><span id="select" class="btn select">SELECT</span>
    <div id="axes" class="axes">axes: —</div>
  </div></section>
  <section class="card"><h2>Game Boy mapping</h2><ul class="mapping"><li><strong>D-pad / left stick</strong> → movement</li><li><strong>Circle / Xbox B</strong> → Game Boy A</li><li><strong>Cross / Xbox A</strong> → Game Boy B</li><li><strong>Square / Xbox X</strong> → auxiliary Y action</li><li><strong>Triangle / Xbox Y</strong> → auxiliary X action</li><li><strong>Options / Menu</strong> → Start</li><li><strong>Create / View</strong> → Select</li></ul></section>
  <section class="card"><h2>Test instructions</h2><ol class="mapping"><li>Use the full Chrome browser, not the captive-portal window.</li><li>Pair a DualSense or Xbox controller with the phone.</li><li>Press <strong>Activate controller</strong>.</li><li>Press buttons and verify the live display.</li></ol></section>
</div><p class="foot">Local and private · No internet or native app</p>
</main><script>
const $=id=>document.getElementById(id), hasAPI=typeof navigator.getGamepads==='function';
const secure=$('secure'),api=$('api'),nameEl=$('name'),mapEl=$('mapping'),log=$('log'),batteryEl=$('battery');
secure.textContent=window.isSecureContext?'Secure context':'Local HTTP';secure.className='value '+(window.isSecureContext?'ok':'warn');
api.textContent=hasAPI?'Available':'Unavailable';api.className='value '+(hasAPI?'ok':'bad');
let active=false,lastId='';
function note(s){log.textContent=s}
function button(g,n){return !!(g&&g.buttons[n]&&g.buttons[n].pressed)}
function set(id,on){$(id).classList.toggle('on',!!on)}
function clearPad(){['up','down','left','right','ba','bb','bx','by','start','select'].forEach(id=>set(id,false));$('axes').textContent='axes: —'}
function frame(){
  const pads=hasAPI?Array.from(navigator.getGamepads()).filter(Boolean):[],g=pads[0];
  if(g){
    nameEl.textContent=g.id||'Gamepad';mapEl.textContent=g.mapping||'non-standard';
    if(g.id!==lastId){lastId=g.id;note('Connected: '+g.id+'\nPress buttons to verify the standard mapping.');}
    const ax=g.axes||[],x=ax[0]||0,y=ax[1]||0;
    set('up',button(g,12)||y<-.45);set('down',button(g,13)||y>.45);set('left',button(g,14)||x<-.45);set('right',button(g,15)||x>.45);
    set('ba',button(g,1));set('bb',button(g,0));set('bx',button(g,3));set('by',button(g,2));set('select',button(g,8));set('start',button(g,9));
    $('axes').textContent='axes: '+ax.map(v=>Number(v).toFixed(2)).join('  ');
  }else if(active){nameEl.textContent='None';mapEl.textContent='—';clearPad();}
  requestAnimationFrame(frame);
}
$('activate').addEventListener('click',()=>{active=true;if(!hasAPI){note('Chrome did not expose navigator.getGamepads on this page. This is likely a browser security restriction.');return}note('Listening… Now press a button on the paired controller.');const p=Array.from(navigator.getGamepads()).filter(Boolean);if(!p.length)nameEl.textContent='Waiting for input…'});
async function refreshBattery(){const c=new AbortController(),timeout=setTimeout(()=>c.abort(),2500);try{const s=await fetch('/api/status',{cache:'no-store',signal:c.signal}).then(r=>r.json());if(s.batteryLevel<0){batteryEl.textContent='Unavailable';batteryEl.className='value warn'}else{batteryEl.textContent=s.batteryLevel+'%'+(s.charging?' · Charging':'')+' · '+(s.batteryVoltageMv/1000).toFixed(2)+' V';batteryEl.className='value '+(s.batteryLevel<=15?'bad':s.batteryLevel<=35?'warn':'ok')}}catch(error){batteryEl.textContent='Unavailable';batteryEl.className='value warn'}finally{clearTimeout(timeout);setTimeout(refreshBattery,5000)}}
window.addEventListener('gamepadconnected',e=>{active=true;note('Connected: '+e.gamepad.id)});window.addEventListener('gamepaddisconnected',e=>{note('Disconnected: '+e.gamepad.id);lastId=''});refreshBattery();frame();
</script></body></html>
)HTML";

const char kPlayerPage[] PROGMEM = R"HTML(
<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#10151f"><title>PocketBoy S3 Player</title><style>
:root{color-scheme:dark;--ink:#f5f7ff;--muted:#aab4ca;--panel:#171d2b;--line:#303a51;--green:#67e3a5;--pink:#ff7096;--blue:#79aaff}
*{box-sizing:border-box}html,body{margin:0;min-height:100%;background:#090c13;color:var(--ink);font:15px system-ui,-apple-system,sans-serif}body{touch-action:manipulation;background:radial-gradient(circle at 50% 0,#29344e,#10151f 45%,#090c13)}
main{width:min(880px,100%);margin:auto;padding:12px}.top{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:10px}.brand{font-size:22px;font-weight:950}.brand span{color:var(--green)}.toplinks{display:flex;align-items:center;gap:10px}.battery{color:var(--green);font-size:12px;font-weight:800}.battery.low{color:#ff6b7a}.battery.mid{color:#ffd166}.back{color:var(--muted);text-decoration:none}.shell{display:grid;grid-template-columns:minmax(0,1fr) 250px;gap:12px}.screenCard,.side{border:1px solid var(--line);border-radius:20px;background:#151a28dd;padding:12px}.screen{position:relative;display:grid;place-items:center;aspect-ratio:160/144;max-height:calc(100vh - 135px);margin:auto;border:8px solid #2b3345;border-radius:12px;background:#050607;overflow:hidden}.screen canvas{width:100%;height:100%;object-fit:contain;image-rendering:pixelated}.empty{position:absolute;text-align:center;color:var(--muted);padding:20px}.empty strong{display:block;color:var(--ink);font-size:20px;margin-bottom:6px}.controls{display:grid;gap:9px}.file{display:block;padding:13px;border-radius:13px;background:var(--green);color:#06150e;font-weight:900;text-align:center;cursor:pointer}.file input{display:none}button{border:0;border-radius:12px;padding:11px;background:#34405a;color:#fff;font:inherit;font-weight:800}button:disabled{opacity:.42;filter:saturate(.25)}.row{display:grid;grid-template-columns:1fr 1fr;gap:8px}.status{padding:10px;border-radius:12px;background:#0d111b;color:var(--muted);font-size:12px;overflow-wrap:anywhere}.status.connected{color:var(--green)}.library{display:grid;gap:6px;max-height:150px;overflow-y:auto}.libraryItem{display:grid;grid-template-columns:minmax(0,1fr) auto;gap:7px;align-items:center;padding:7px;border-radius:10px;background:#0d111b}.libraryName{min-width:0;color:var(--ink);font-size:12px;font-weight:800;overflow-wrap:anywhere}.librarySize{display:block;color:var(--muted);font-size:10px;font-weight:500}.libraryActions{display:flex;gap:5px}.libraryActions button{padding:7px 8px;border-radius:8px;font-size:11px}.libraryActions .deleteGame{background:#633342}label.range{display:grid;gap:6px;color:var(--muted);font-size:12px}input[type=range]{width:100%}.note{color:var(--muted);font-size:12px;line-height:1.45}.touch{display:none;position:relative;height:210px;margin-top:10px;user-select:none;-webkit-user-select:none;touch-action:none}.dpad{position:absolute;left:8px;bottom:4px;width:170px;height:170px}.dpad div{position:absolute;background:#3d465d}.dpad:after{content:'';position:absolute;left:61px;top:61px;width:48px;height:48px;background:#3d465d}.left,.right{top:61px;width:61px;height:48px}.left{left:0;border-radius:10px 0 0 10px}.right{right:0;border-radius:0 10px 10px 0}.up,.down{left:61px;width:48px;height:61px}.up{top:0;border-radius:10px 10px 0 0}.down{bottom:0;border-radius:0 0 10px 10px}.roundBtn,.capsuleBtn{position:absolute;display:grid;place-items:center;background:var(--pink);font-weight:950}.roundBtn{width:66px;height:66px;border-radius:50%;font-size:24px}.capsuleBtn{width:66px;height:30px;border-radius:20px;background:#46516b;font-size:10px}.btnPressed{filter:brightness(1.6);transform:scale(.95)}#controller_a{right:9px;bottom:91px}#controller_b{right:88px;bottom:57px}#controller_start{right:8px;bottom:4px}#controller_select{right:83px;bottom:4px}
@media(max-width:700px){.shell{grid-template-columns:1fr}.screen{max-height:none}.touch{display:block}.side{padding-bottom:8px}}@media(orientation:landscape) and (max-height:520px){main{width:100%;padding:6px}.top{margin:0 4px 5px}.shell{grid-template-columns:minmax(0,1fr) 230px}.screenCard{padding:6px}.screen{height:calc(100vh - 55px);width:auto}.touch{display:none}.side{padding:8px;max-height:calc(100vh - 55px);overflow-y:auto}.note{display:none}}
body.game-fullscreen{overflow:hidden}body.game-fullscreen .screen,.screen:fullscreen{position:fixed;z-index:9999;inset:0;display:block;width:100vw;height:100vh;max-height:none;aspect-ratio:auto;margin:0;border:0;border-radius:0;background:#000;overflow:hidden;transform:none}body.game-fullscreen .screen canvas,.screen:fullscreen canvas{display:block;width:100%;height:100%;object-fit:contain}body.game-fullscreen .touch{display:none}@supports(width:100dvw){body.game-fullscreen .screen,.screen:fullscreen{width:100dvw;height:100dvh}}@media(orientation:portrait){body.game-fullscreen .screen canvas,.screen:fullscreen canvas{position:absolute;left:50%;top:50%;width:100vh;height:100vw;max-width:none;max-height:none;transform:translate(-50%,-50%) rotate(90deg);transform-origin:center center}@supports(width:100dvw){body.game-fullscreen .screen canvas,.screen:fullscreen canvas{width:100dvh;height:100dvw}}}
</style></head><body><main><header class="top"><div class="brand">Pocket<span>Boy</span> S3</div><div class="toplinks"><span id="batteryStatus" class="battery">Battery —</span><a class="back" href="/">Controller lab</a></div></header>
<div class="shell"><section class="screenCard"><div class="screen"><canvas id="mainCanvas" width="160" height="144"></canvas><div id="empty" class="empty"><strong>Select a game</strong>Open one of your homebrew .gb or .gbc files from this phone.</div></div>
<div id="controller" class="touch"><div id="controller_dpad" class="dpad"><div id="controller_left" class="left"></div><div id="controller_right" class="right"></div><div id="controller_up" class="up"></div><div id="controller_down" class="down"></div></div><div id="controller_select" class="capsuleBtn">Select</div><div id="controller_start" class="capsuleBtn">Start</div><div id="controller_b" class="roundBtn">B</div><div id="controller_a" class="roundBtn">A</div></div></section>
<aside class="side"><div class="controls"><label class="file">Open .gb / .gbc<input id="rom" type="file" accept=".gb,.gbc,application/octet-stream"></label><div class="row"><button type="button" id="uploadGame">Upload game</button><button type="button" id="refreshGames">Refresh library</button></div><input id="gameUploadFile" type="file" accept=".gb,.gbc,application/octet-stream" hidden><div id="gameStorage" class="status">Game library: checking…</div><div id="gameLibrary" class="library"><div class="status">Loading games…</div></div><div id="romName" class="status">No game loaded</div><div id="gamepadStatus" class="status">No controller detected</div><div class="row"><button type="button" id="sound">Sound On</button><button type="button" id="fullscreen">Enter fullscreen</button></div><div class="row"><button type="button" id="pause">Pause</button><button type="button" id="save" disabled>Save state</button></div><button type="button" id="load" disabled>Load state</button><div class="row"><button type="button" id="exportSaves">Export saves</button><button type="button" id="importSaves">Import saves</button></div><div class="row"><button type="button" id="syncS3">Sync to S3</button><button type="button" id="restoreS3">Restore S3</button></div><div id="s3SaveStatus" class="status">S3 save space: checking…</div><input id="saveBackupFile" type="file" accept=".json,application/json" hidden><label class="range">Volume<input id="volume" type="range" min="0" max="1" value="0.5" step="0.05"></label><p class="note">Store legally obtained homebrew GB/GBC games on the S3. Manual save export remains the safest backup before erasing flash.</p><div id="message" class="status">PocketBoy v0.6.0 ready. Choose a game.</div></div></aside></div></main>
<script src="/binjgb.js?v=060"></script><script src="/player.js?v=060"></script><script>
const rom=document.getElementById('rom'),msg=document.getElementById('message'),empty=document.getElementById('empty'),saveButton=document.getElementById('save'),loadButton=document.getElementById('load');let stateUnlockTimer=0;
function lockStateControls(){clearTimeout(stateUnlockTimer);saveButton.disabled=true;loadButton.disabled=true}
function unlockStateControlsWhenReady(){const wait=PocketBoyPlayer.stateReadyInMs();if(wait===null)return;if(wait>0){stateUnlockTimer=setTimeout(unlockStateControlsWhenReady,wait+20);return}saveButton.disabled=false;loadButton.disabled=false;msg.textContent='Running. Save and Load are ready.'}
const batteryStatus=document.getElementById('batteryStatus');async function refreshBattery(){const c=new AbortController(),timeout=setTimeout(()=>c.abort(),2500);try{const s=await fetch('/api/status',{cache:'no-store',signal:c.signal}).then(r=>r.json());if(s.batteryLevel<0){batteryStatus.textContent='Battery unavailable';batteryStatus.className='battery mid'}else{batteryStatus.textContent=s.batteryLevel+'%'+(s.charging?' ⚡':'');batteryStatus.className='battery '+(s.batteryLevel<=15?'low':s.batteryLevel<=35?'mid':'')}}catch(error){batteryStatus.textContent='Battery unavailable';batteryStatus.className='battery mid'}finally{clearTimeout(timeout);setTimeout(refreshBattery,5000)}}refreshBattery();
async function startRom(buffer,name){lockStateControls();try{msg.textContent='Loading '+name+'…';await PocketBoyPlayer.start(buffer);document.getElementById('romName').textContent=name;document.getElementById('pause').textContent='Pause';empty.style.display='none';msg.textContent='Running. Save and Load unlock in 1.5 seconds.';unlockStateControlsWhenReady()}catch(error){console.error(error);msg.textContent='Could not start this ROM: '+error.message}}
rom.addEventListener('change',async()=>{const file=rom.files&&rom.files[0];if(!file)return;if(!/\.(gb|gbc)$/i.test(file.name)){msg.textContent='Please choose a .gb or .gbc file.';return}await startRom(await file.arrayBuffer(),file.name)});
const gameUploadFile=document.getElementById('gameUploadFile'),gameLibrary=document.getElementById('gameLibrary'),gameStorage=document.getElementById('gameStorage');
async function loadStoredGame(name){try{msg.textContent='Reading '+name+' from the S3…';const response=await fetch('/api/games/rom?name='+encodeURIComponent(name),{cache:'no-store'});if(!response.ok)throw new Error('The stored game could not be read.');await startRom(await response.arrayBuffer(),name)}catch(error){msg.textContent='Game load failed: '+error.message}}
async function deleteStoredGame(name){if(!confirm('Delete '+name+' from the S3 game library? Its Chrome and S3 save backups will not be deleted.'))return;try{const response=await fetch('/api/games?name='+encodeURIComponent(name),{method:'DELETE'}),result=await response.json();if(!response.ok)throw new Error(result.error||'The game could not be deleted.');msg.textContent='Deleted '+name+' from the S3. Any save data was kept.';await refreshGameLibrary()}catch(error){msg.textContent='Delete failed: '+error.message}}
function renderGameLibrary(status){gameLibrary.replaceChildren();const games=status.games||[];games.sort((a,b)=>a.name.localeCompare(b.name));if(!games.length){const item=document.createElement('div');item.className='status';item.textContent='No stored games yet.';gameLibrary.append(item);return}for(const game of games){const item=document.createElement('div');item.className='libraryItem';const label=document.createElement('div');label.className='libraryName';label.textContent=game.name;const size=document.createElement('span');size.className='librarySize';size.textContent=formatBytes(game.size);label.append(size);const actions=document.createElement('div');actions.className='libraryActions';const play=document.createElement('button');play.type='button';play.textContent='Play';play.onclick=()=>loadStoredGame(game.name);const del=document.createElement('button');del.type='button';del.textContent='Delete';del.className='deleteGame';del.onclick=()=>deleteStoredGame(game.name);actions.append(play,del);item.append(label,actions);gameLibrary.append(item)}}
async function refreshGameLibrary(){try{const response=await fetch('/api/games',{cache:'no-store'}),status=await response.json();if(!response.ok||!status.ready)throw new Error('unavailable');gameStorage.textContent='Game library: '+formatBytes(status.usedBytes)+' used · '+formatBytes(status.freeBytes)+' free';renderGameLibrary(status);return status}catch(error){gameStorage.textContent='Game library unavailable';gameLibrary.replaceChildren();const item=document.createElement('div');item.className='status';item.textContent='Could not read stored games.';gameLibrary.append(item);return null}}
document.getElementById('refreshGames').onclick=refreshGameLibrary;document.getElementById('uploadGame').onclick=()=>{gameUploadFile.value='';gameUploadFile.click()};gameUploadFile.onchange=async()=>{const file=gameUploadFile.files&&gameUploadFile.files[0];if(!file)return;try{if(!/\.(gb|gbc)$/i.test(file.name))throw new Error('Choose a .gb or .gbc file.');const status=await refreshGameLibrary();if(!status)throw new Error('Game storage is unavailable.');if(file.size>status.freeBytes)throw new Error('This game is larger than the available S3 space.');if(!confirm('Upload '+file.name+' ('+formatBytes(file.size)+') to the S3 game library?')){msg.textContent='Game upload cancelled.';return}msg.textContent='Uploading '+file.name+' to the S3…';const form=new FormData();form.append('game',file,file.name);const response=await fetch('/api/games',{method:'POST',body:form}),result=await response.json();if(!response.ok)throw new Error(result.error||'The S3 rejected this game.');msg.textContent='Added '+result.name+' to the S3 game library.';await refreshGameLibrary()}catch(error){msg.textContent='Game upload failed: '+error.message}};refreshGameLibrary();
document.getElementById('pause').onclick=()=>{try{const paused=PocketBoyPlayer.togglePause();if(paused===null){msg.textContent='Load a game before using Pause.';return}document.getElementById('pause').textContent=paused?'Resume':'Pause';msg.textContent=paused?'Game paused.':'Game resumed.'}catch(error){msg.textContent='Pause failed: '+error.message}};
saveButton.onclick=()=>{try{msg.textContent=PocketBoyPlayer.saveState()?'Save state stored in this browser.':'Save State is not ready yet.'}catch(error){msg.textContent='Save failed: '+error.message}};loadButton.onclick=async()=>{try{msg.textContent='Loading save state…';msg.textContent=await PocketBoyPlayer.loadState()?'Save state loaded.':'No compatible save state is ready for this game.'}catch(error){msg.textContent='Load failed: '+error.message}};document.getElementById('volume').oninput=e=>PocketBoyPlayer.setVolume(e.target.value);
const backupFile=document.getElementById('saveBackupFile'),s3SaveStatus=document.getElementById('s3SaveStatus');
function resetAfterSaveImport(){lockStateControls();rom.value='';document.getElementById('romName').textContent='No game loaded';document.getElementById('pause').textContent='Pause';empty.style.display='block'}
function importBackupText(text,source){const info=PocketBoyPlayer.inspectSaveBackup(text);if(!confirm('Restore '+info.entryCount+' save records for '+info.romCount+' games from '+source+'? Matching saves in Chrome will be replaced and the current game will close.'))return null;const result=PocketBoyPlayer.importSaveBackup(text);resetAfterSaveImport();return result}
function formatBytes(value){if(value<1024)return value+' B';return(value/1024).toFixed(value<10240?1:0)+' KiB'}
async function makeS3BackupBlob(text){if(typeof CompressionStream==='undefined')return new Blob([text],{type:'application/json'});const stream=new Blob([text]).stream().pipeThrough(new CompressionStream('gzip'));return await new Response(stream).blob()}
async function readS3Backup(response){const data=await response.arrayBuffer(),bytes=new Uint8Array(data);if(bytes.length>1&&bytes[0]===0x1f&&bytes[1]===0x8b){if(typeof DecompressionStream==='undefined')throw new Error('This Chrome version cannot decompress the S3 backup.');const stream=new Blob([data]).stream().pipeThrough(new DecompressionStream('gzip'));return await new Response(stream).text()}return new TextDecoder().decode(bytes)}
async function refreshS3SaveStatus(){try{const response=await fetch('/api/saves',{cache:'no-store'}),status=await response.json();if(!response.ok||!status.ready)throw new Error('unavailable');s3SaveStatus.textContent=status.hasBackup?'S3 backup: '+formatBytes(status.backupSize)+' · '+formatBytes(status.usedBytes)+' of '+formatBytes(status.totalBytes)+' used':'S3 backup: empty · '+formatBytes(status.totalBytes)+' reserved';return status}catch(error){s3SaveStatus.textContent='S3 save space unavailable';return null}}
document.getElementById('exportSaves').onclick=()=>{try{const backup=PocketBoyPlayer.exportSaveBackup();if(!backup.entryCount){msg.textContent='There are no PocketBoy saves to export yet.';return}const url=URL.createObjectURL(new Blob([backup.text],{type:'application/json'})),link=document.createElement('a');link.href=url;link.download=backup.filename;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);msg.textContent='Exported '+backup.entryCount+' save records for '+backup.romCount+' games.'}catch(error){msg.textContent='Export failed: '+error.message}};
document.getElementById('importSaves').onclick=()=>{backupFile.value='';backupFile.click()};backupFile.onchange=async()=>{const file=backupFile.files&&backupFile.files[0];if(!file)return;try{if(file.size>64*1024*1024)throw new Error('The backup file is too large.');const result=importBackupText(await file.text(),'this file');msg.textContent=result?'Imported '+result.entryCount+' save records for '+result.romCount+' games. Reopen a ROM to use them.':'Save import cancelled.'}catch(error){msg.textContent='Import failed: '+error.message}};
document.getElementById('syncS3').onclick=async()=>{try{const backup=PocketBoyPlayer.exportSaveBackup();if(!backup.entryCount){msg.textContent='There are no PocketBoy saves to sync yet.';return}const payload=await makeS3BackupBlob(backup.text),status=await refreshS3SaveStatus();if(!status)throw new Error('S3 save space is unavailable.');if(payload.size>status.maxBackupBytes)throw new Error('The compressed backup is '+formatBytes(payload.size)+'; the safe S3 limit is '+formatBytes(status.maxBackupBytes)+'.');if(!confirm('Store '+backup.entryCount+' save records for '+backup.romCount+' games on the S3? This replaces its previous backup.')){msg.textContent='S3 sync cancelled.';return}msg.textContent='Syncing saves to the S3…';const form=new FormData();form.append('backup',payload,backup.filename+(payload.type==='application/json'?'':'.gz'));const response=await fetch('/api/saves/backup',{method:'POST',body:form}),result=await response.json();if(!response.ok)throw new Error(result.error||'The S3 rejected the backup.');msg.textContent='Synced '+backup.entryCount+' save records to the S3 ('+formatBytes(payload.size)+').';await refreshS3SaveStatus()}catch(error){msg.textContent='S3 sync failed: '+error.message}};
document.getElementById('restoreS3').onclick=async()=>{try{const status=await refreshS3SaveStatus();if(!status||!status.hasBackup){msg.textContent='There is no backup stored on the S3 yet.';return}msg.textContent='Reading the S3 backup…';const response=await fetch('/api/saves/backup',{cache:'no-store'});if(!response.ok)throw new Error('The S3 backup could not be read.');const result=importBackupText(await readS3Backup(response),'the S3');msg.textContent=result?'Restored '+result.entryCount+' save records for '+result.romCount+' games. Reopen a ROM to use them.':'S3 restore cancelled.'}catch(error){msg.textContent='S3 restore failed: '+error.message}};refreshS3SaveStatus();
document.getElementById('sound').onclick=()=>{const enabled=PocketBoyPlayer.toggleSound();document.getElementById('sound').textContent=enabled?'Sound Off':'Sound On';msg.textContent=enabled?'Sound enabled.':'Sound muted.'};
const fullButton=document.getElementById('fullscreen'),fullTarget=document.querySelector('.screen');let pocketBoyScreenMode=false;
async function enterScreenMode(){if(!PocketBoyPlayer.hasGame()){msg.textContent='Load a game before entering fullscreen.';return}pocketBoyScreenMode=true;document.body.classList.add('game-fullscreen');fullButton.textContent='Exit fullscreen';msg.textContent='Controller screen mode active. Press L2 to exit.';try{if(fullTarget.requestFullscreen)await fullTarget.requestFullscreen({navigationUI:'hide'});else if(fullTarget.webkitRequestFullscreen)fullTarget.webkitRequestFullscreen()}catch(error){}if(document.fullscreenElement&&screen.orientation&&screen.orientation.lock){try{await screen.orientation.lock('landscape')}catch(error){}}}
async function exitScreenMode(){if(!pocketBoyScreenMode&&!document.fullscreenElement)return;pocketBoyScreenMode=false;document.body.classList.remove('game-fullscreen');fullButton.textContent='Enter fullscreen';if(screen.orientation&&screen.orientation.unlock)screen.orientation.unlock();if(document.fullscreenElement&&document.exitFullscreen){try{await document.exitFullscreen()}catch(error){}}}
function toggleScreenMode(){if(pocketBoyScreenMode||document.fullscreenElement)exitScreenMode();else enterScreenMode()}
window.PocketBoyFullscreen={toggle:toggleScreenMode,exit:exitScreenMode};fullButton.onclick=toggleScreenMode;
</script></body></html>
)HTML";
}

void WebPortal::begin() {
    const uint64_t chipId = ESP.getEfuseMac();
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%04X", static_cast<uint16_t>(chipId));
    char secret[13];
    snprintf(secret, sizeof(secret), "Boy%08X", static_cast<uint32_t>(chipId));
    ssid_ = String("PocketBoy-") + suffix;
    password_ = secret;

    saveStorageReady_ = saveFs_.begin(true, "/pbsaves", 4, "saves");
    if (saveStorageReady_) {
        saveFs_.remove(kSaveUploadPath);
        if (!saveFs_.exists(kSaveBackupPath) && saveFs_.exists(kSavePreviousPath)) {
            saveFs_.rename(kSavePreviousPath, kSaveBackupPath);
        } else {
            saveFs_.remove(kSavePreviousPath);
        }
    }
    gameStorageReady_ = gameFs_.begin(true, "/pbgames", 6, "games");
    if (gameStorageReady_) gameFs_.remove(kGameUploadPath);

    WiFi.mode(WIFI_AP);
    WiFi.setSleep(true);
    WiFi.softAP(ssid_.c_str(), password_.c_str());
    if (MDNS.begin("pocketboy")) MDNS.addService("http", "tcp", 80);
    dns_.start(kDnsPort, "*", WiFi.softAPIP());
    configureRoutes();
    server_.begin();
}

void WebPortal::loop() {
    dns_.processNextRequest();
    server_.handleClient();
}

const String &WebPortal::ssid() const { return ssid_; }
const String &WebPortal::password() const { return password_; }
uint8_t WebPortal::connectedClients() const { return WiFi.softAPgetStationNum(); }

void WebPortal::sendStatus() {
    const int32_t batteryLevel = M5.Power.getBatteryLevel();
    const int16_t batteryVoltage = M5.Power.getBatteryVoltage();
    const auto chargeState = M5.Power.isCharging();
    String json = "{\"clients\":" + String(connectedClients());
    json += ",\"uptimeSeconds\":" + String(millis() / 1000);
    json += ",\"freeHeap\":" + String(ESP.getFreeHeap());
    json += ",\"batteryLevel\":" + String(batteryLevel);
    json += ",\"batteryVoltageMv\":" + String(batteryVoltage);
    json += ",\"charging\":" + String(chargeState == m5::Power_Class::is_charging ? "true" : "false");
    json += ",\"chargingKnown\":" + String(chargeState == m5::Power_Class::charge_unknown ? "false" : "true") + "}";
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(200, "application/json", json);
}

void WebPortal::sendSaveStorageStatus() {
    String json = "{\"ready\":" + String(saveStorageReady_ ? "true" : "false");
    if (saveStorageReady_) {
        const bool hasBackup = saveFs_.exists(kSaveBackupPath);
        size_t backupSize = 0;
        if (hasBackup) {
            File file = saveFs_.open(kSaveBackupPath, FILE_READ);
            if (file) backupSize = file.size();
        }
        json += ",\"hasBackup\":" + String(hasBackup ? "true" : "false");
        json += ",\"backupSize\":" + String(backupSize);
        json += ",\"usedBytes\":" + String(saveFs_.usedBytes());
        json += ",\"totalBytes\":" + String(saveFs_.totalBytes());
        json += ",\"maxBackupBytes\":" + String(kMaxStoredBackupBytes);
    }
    json += "}";
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(saveStorageReady_ ? 200 : 503, "application/json", json);
}

void WebPortal::sendSaveBackup() {
    if (!saveStorageReady_) {
        server_.send(503, "application/json", "{\"error\":\"Save storage is unavailable.\"}");
        return;
    }
    File file = saveFs_.open(kSaveBackupPath, FILE_READ);
    if (!file) {
        server_.send(404, "application/json", "{\"error\":\"No S3 backup has been stored yet.\"}");
        return;
    }
    server_.sendHeader("Cache-Control", "no-store");
    server_.sendHeader("Content-Disposition", "attachment; filename=\"pocketboy-s3-saves.json\"");
    server_.streamFile(file, "application/json");
    file.close();
}

void WebPortal::handleSaveUpload() {
    HTTPUpload &upload = server_.upload();
    if (upload.status == UPLOAD_FILE_START) {
        saveUploadOk_ = saveStorageReady_;
        saveUploadBytes_ = 0;
        saveUploadError_ = saveStorageReady_ ? "" : "Save storage is unavailable.";
        if (saveUpload_) saveUpload_.close();
        if (saveStorageReady_) {
            saveFs_.remove(kSaveUploadPath);
            saveUpload_ = saveFs_.open(kSaveUploadPath, FILE_WRITE);
            if (!saveUpload_) {
                saveUploadOk_ = false;
                saveUploadError_ = "Could not create the temporary backup.";
            }
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (!saveUploadOk_) return;
        if (saveUploadBytes_ + upload.currentSize > kMaxStoredBackupBytes) {
            saveUploadOk_ = false;
            saveUploadError_ = "The backup exceeds the S3 save-space limit.";
            saveUpload_.close();
            saveFs_.remove(kSaveUploadPath);
            return;
        }
        if (saveUpload_.write(upload.buf, upload.currentSize) != upload.currentSize) {
            saveUploadOk_ = false;
            saveUploadError_ = "The S3 could not finish writing the backup.";
            saveUpload_.close();
            saveFs_.remove(kSaveUploadPath);
            return;
        }
        saveUploadBytes_ += upload.currentSize;
    } else if (upload.status == UPLOAD_FILE_END) {
        if (saveUpload_) saveUpload_.close();
        if (!saveUploadOk_ || saveUploadBytes_ == 0) {
            if (saveUploadError_.isEmpty()) saveUploadError_ = "The uploaded backup was empty.";
            saveUploadOk_ = false;
            saveFs_.remove(kSaveUploadPath);
            return;
        }

        saveFs_.remove(kSavePreviousPath);
        const bool hadBackup = saveFs_.exists(kSaveBackupPath);
        if (hadBackup && !saveFs_.rename(kSaveBackupPath, kSavePreviousPath)) {
            saveUploadOk_ = false;
            saveUploadError_ = "The existing S3 backup could not be protected.";
            saveFs_.remove(kSaveUploadPath);
            return;
        }
        if (!saveFs_.rename(kSaveUploadPath, kSaveBackupPath)) {
            saveUploadOk_ = false;
            saveUploadError_ = "The new S3 backup could not be activated.";
            if (hadBackup) saveFs_.rename(kSavePreviousPath, kSaveBackupPath);
            saveFs_.remove(kSaveUploadPath);
            return;
        }
        saveFs_.remove(kSavePreviousPath);
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (saveUpload_) saveUpload_.close();
        saveFs_.remove(kSaveUploadPath);
        saveUploadOk_ = false;
        saveUploadError_ = "The backup upload was interrupted.";
    }
}

void WebPortal::finishSaveUpload() {
    if (!saveUploadOk_) {
        String json = "{\"error\":\"" + saveUploadError_ + "\"}";
        server_.send(507, "application/json", json);
        saveUploadBytes_ = 0;
        return;
    }
    String json = "{\"stored\":true,\"backupSize\":" + String(saveUploadBytes_) + "}";
    saveUploadOk_ = false;
    saveUploadBytes_ = 0;
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(201, "application/json", json);
}

void WebPortal::sendGameLibrary() {
    String json = "{\"ready\":" + String(gameStorageReady_ ? "true" : "false");
    if (gameStorageReady_) {
        const size_t totalBytes = gameFs_.totalBytes();
        const size_t usedBytes = gameFs_.usedBytes();
        json += ",\"usedBytes\":" + String(usedBytes);
        json += ",\"totalBytes\":" + String(totalBytes);
        json += ",\"freeBytes\":" + String(totalBytes > usedBytes ? totalBytes - usedBytes : 0);
        json += ",\"maxGameBytes\":" + String(kMaxStoredGameBytes);
        json += ",\"games\":[";
        bool first = true;
        File root = gameFs_.open("/");
        if (root) {
            File file = root.openNextFile();
            while (file) {
                String name = file.name();
                if (name.startsWith("/")) name.remove(0, 1);
                if (!file.isDirectory() && isValidGameName(name)) {
                    if (!first) json += ',';
                    first = false;
                    json += "{\"name\":\"" + jsonEscape(name) + "\",\"size\":" + String(file.size()) + "}";
                }
                file.close();
                file = root.openNextFile();
            }
            root.close();
        }
        json += ']';
    }
    json += '}';
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(gameStorageReady_ ? 200 : 503, "application/json", json);
}

void WebPortal::sendStoredGame() {
    if (!gameStorageReady_) {
        server_.send(503, "application/json", "{\"error\":\"Game storage is unavailable.\"}");
        return;
    }
    const String name = server_.arg("name");
    if (!isValidGameName(name)) {
        server_.send(400, "application/json", "{\"error\":\"Invalid game name.\"}");
        return;
    }
    File file = gameFs_.open(String("/") + name, FILE_READ);
    if (!file) {
        server_.send(404, "application/json", "{\"error\":\"Game not found.\"}");
        return;
    }
    server_.sendHeader("Cache-Control", "no-store");
    server_.sendHeader("Content-Disposition", "attachment; filename=\"" + name + "\"");
    server_.streamFile(file, "application/octet-stream");
    file.close();
}

void WebPortal::handleGameUpload() {
    HTTPUpload &upload = server_.upload();
    if (upload.status == UPLOAD_FILE_START) {
        gameUploadOk_ = gameStorageReady_;
        gameUploadBytes_ = 0;
        gameUploadName_ = sanitizeGameName(upload.filename);
        gameUploadError_ = gameStorageReady_ ? "" : "Game storage is unavailable.";
        gameUploadResponseCode_ = gameStorageReady_ ? 400 : 503;
        if (gameUpload_) gameUpload_.close();
        if (!gameStorageReady_) return;
        if (gameUploadName_.isEmpty()) {
            gameUploadOk_ = false;
            gameUploadError_ = "Only .gb and .gbc files can be stored.";
            return;
        }
        if (gameFs_.exists(String("/") + gameUploadName_)) {
            gameUploadOk_ = false;
            gameUploadError_ = "A game with this name already exists. Delete it before replacing it.";
            gameUploadResponseCode_ = 409;
            return;
        }
        gameFs_.remove(kGameUploadPath);
        gameUpload_ = gameFs_.open(kGameUploadPath, FILE_WRITE);
        if (!gameUpload_) {
            gameUploadOk_ = false;
            gameUploadError_ = "Could not create the temporary game file.";
            gameUploadResponseCode_ = 507;
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (!gameUploadOk_) return;
        if (gameUploadBytes_ + upload.currentSize > kMaxStoredGameBytes) {
            gameUploadOk_ = false;
            gameUploadError_ = "This ROM is too large for the PocketBoy game library.";
            gameUploadResponseCode_ = 413;
            gameUpload_.close();
            gameFs_.remove(kGameUploadPath);
            return;
        }
        if (gameUpload_.write(upload.buf, upload.currentSize) != upload.currentSize) {
            gameUploadOk_ = false;
            gameUploadError_ = "There is not enough free game-library space.";
            gameUploadResponseCode_ = 507;
            gameUpload_.close();
            gameFs_.remove(kGameUploadPath);
            return;
        }
        gameUploadBytes_ += upload.currentSize;
    } else if (upload.status == UPLOAD_FILE_END) {
        if (gameUpload_) gameUpload_.close();
        if (!gameUploadOk_ || gameUploadBytes_ == 0) {
            if (gameUploadError_.isEmpty()) gameUploadError_ = "The uploaded game was empty.";
            gameUploadOk_ = false;
            gameFs_.remove(kGameUploadPath);
            return;
        }
        if (!gameFs_.rename(kGameUploadPath, String("/") + gameUploadName_)) {
            gameUploadOk_ = false;
            gameUploadError_ = "The uploaded game could not be added to the library.";
            gameUploadResponseCode_ = 500;
            gameFs_.remove(kGameUploadPath);
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (gameUpload_) gameUpload_.close();
        gameFs_.remove(kGameUploadPath);
        gameUploadOk_ = false;
        gameUploadError_ = "The game upload was interrupted.";
        gameUploadResponseCode_ = 400;
    }
}

void WebPortal::finishGameUpload() {
    if (!gameUploadOk_) {
        const String json = "{\"error\":\"" + jsonEscape(gameUploadError_) + "\"}";
        server_.send(gameUploadResponseCode_, "application/json", json);
        gameUploadBytes_ = 0;
        return;
    }
    const String json = "{\"stored\":true,\"name\":\"" + jsonEscape(gameUploadName_) +
        "\",\"size\":" + String(gameUploadBytes_) + "}";
    gameUploadOk_ = false;
    gameUploadBytes_ = 0;
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(201, "application/json", json);
}

void WebPortal::deleteStoredGame() {
    if (!gameStorageReady_) {
        server_.send(503, "application/json", "{\"error\":\"Game storage is unavailable.\"}");
        return;
    }
    const String name = server_.arg("name");
    if (!isValidGameName(name)) {
        server_.send(400, "application/json", "{\"error\":\"Invalid game name.\"}");
        return;
    }
    const String path = String("/") + name;
    if (!gameFs_.exists(path)) {
        server_.send(404, "application/json", "{\"error\":\"Game not found.\"}");
        return;
    }
    if (!gameFs_.remove(path)) {
        server_.send(500, "application/json", "{\"error\":\"The game could not be deleted.\"}");
        return;
    }
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(200, "application/json", "{\"deleted\":true}");
}

void WebPortal::configureRoutes() {
    server_.on("/", HTTP_GET, [this]() {
        server_.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
        server_.send_P(200, "text/html", kPage);
    });
    server_.on("/play", HTTP_GET, [this]() {
        server_.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
        server_.send_P(200, "text/html", kPlayerPage);
    });
    server_.on("/binjgb.js", HTTP_GET, [this]() {
        server_.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
        server_.send_P(200, "text/javascript", reinterpret_cast<PGM_P>(kBinjgbJs), kBinjgbJsSize);
    });
    server_.on("/player.js", HTTP_GET, [this]() {
        server_.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
        server_.send_P(200, "text/javascript", reinterpret_cast<PGM_P>(kPlayerJs), kPlayerJsSize);
    });
    server_.on("/binjgb.wasm", HTTP_GET, [this]() {
        server_.sendHeader("Cache-Control", "public, max-age=86400");
        server_.send_P(200, "application/wasm", reinterpret_cast<PGM_P>(kBinjgbWasm), kBinjgbWasmSize);
    });
    server_.on("/api/status", HTTP_GET, [this]() { sendStatus(); });
    server_.on("/api/saves", HTTP_GET, [this]() { sendSaveStorageStatus(); });
    server_.on("/api/saves/backup", HTTP_GET, [this]() { sendSaveBackup(); });
    server_.on("/api/saves/backup", HTTP_POST,
        [this]() { finishSaveUpload(); },
        [this]() { handleSaveUpload(); });
    server_.on("/api/games", HTTP_GET, [this]() { sendGameLibrary(); });
    server_.on("/api/games", HTTP_POST,
        [this]() { finishGameUpload(); },
        [this]() { handleGameUpload(); });
    server_.on("/api/games", HTTP_DELETE, [this]() { deleteStoredGame(); });
    server_.on("/api/games/rom", HTTP_GET, [this]() { sendStoredGame(); });
    server_.on("/generate_204", HTTP_GET, [this]() { server_.sendHeader("Location", "/", true); server_.send(302); });
    server_.on("/hotspot-detect.html", HTTP_GET, [this]() { server_.send_P(200, "text/html", kPage); });
    server_.on("/connecttest.txt", HTTP_GET, [this]() { server_.sendHeader("Location", "/", true); server_.send(302); });
    server_.onNotFound([this]() {
        server_.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        server_.send(302, "text/plain", "");
    });
}
