/* Knihovna fotografických pozadí (obloha a krajina 360°) – společná pro Skeet a Střelnici.
   Seznam je v pozadi/seznam.json: nové pozadí = soubor JPG (equirektangulární 2:1) do složky pozadi/ + řádek v seznamu.
   Vlastní fotky 360° se dají nahrát i přímo v prohlížeči (uloží se jen v tomto zařízení, IndexedDB). */
(function(){
  const P = window.POZADI = { list: [], cur: null, mesh: null, tex: null, yaw: 0, onApply: null, scene: null, _load: 0 };
  const IDB = { db: null,
    open(){ return this.db || (this.db = new Promise((res, rej) => { try { const r = indexedDB.open('pozadi', 1); r.onupgradeneeded = () => r.result.createObjectStore('f'); r.onsuccess = () => res(r.result); r.onerror = () => rej(r.error); } catch (e) { rej(e); } })); },
    async op(mode, fn){ const db = await this.open(); return new Promise((res, rej) => { const t = db.transaction('f', mode), s = t.objectStore('f'); const q = fn(s); t.oncomplete = () => res(q && q.result); t.onerror = () => rej(t.error); }); },
    put(k, v){ return this.op('readwrite', s => s.put(v, k)); }, get(k){ return this.op('readonly', s => s.get(k)); },
    del(k){ return this.op('readwrite', s => s.delete(k)); }, keys(){ return this.op('readonly', s => s.getAllKeys()); } };
  P.nacti = async function(){
    let l = [];
    try { const r = await fetch('pozadi/seznam.json', { cache: 'no-cache' }); if (r.ok) l = await r.json(); } catch (e) {}
    try { const ks = await IDB.keys(); for (const k of ks || []) if (String(k).startsWith('v:')) l.push({ id: k, n: '📷 ' + String(k).slice(2), typ: 'krajina', vlastni: true }); } catch (e) {}
    P.list = l; return l; };
  P.pridejVlastni = async function(file){ const k = 'v:' + file.name.replace(/\.[^.]+$/, '').slice(0, 40); await IDB.put(k, file); await P.nacti(); return k; };
  P.smazVlastni = async function(id){ try { await IDB.del(id); } catch (e) {} await P.nacti(); };
  P.init = function(scene, onApply, radius, srgb){ P.scene = scene; P.onApply = onApply; P.R = radius || 1000; P.srgb = srgb !== false; };
  function horizont(img){ try { const c = document.createElement('canvas'); c.width = 64; c.height = 32; const x = c.getContext('2d'); x.drawImage(img, 0, 0, 64, 32);
      const d = x.getImageData(0, 14, 64, 3).data; let r = 0, g = 0, b = 0, n = 0; for (let i = 0; i < d.length; i += 4) { r += d[i]; g += d[i + 1]; b += d[i + 2]; n++; } return [r / n / 255, g / n / 255, b / n / 255]; } catch (e) { return null; } }
  // kde je na fotce slunce (nejjasnější místo nad obzorem) – podle něj se natočí světlo a stíny
  function slunce(img){ try { const W = 128, H = 64, c = document.createElement('canvas'); c.width = W; c.height = H; const x = c.getContext('2d'); x.drawImage(img, 0, 0, W, H);
      const d = x.getImageData(0, 0, W, H / 2).data; let best = 0, bi = -1, sum = 0;
      for (let i = 0; i < W * H / 2; i++) { const l = 0.2126 * d[i * 4] + 0.7152 * d[i * 4 + 1] + 0.0722 * d[i * 4 + 2]; sum += l; if (l > best) { best = l; bi = i; } }
      const avg = sum / (W * H / 2); if (bi < 0 || best < 250 || best < avg * 1.6) return null;
      return [((bi % W) + 0.5) / W, (Math.floor(bi / W) + 0.5) / H]; } catch (e) { return null; } }
  P.vypni = function(){ if (P.mesh) P.mesh.visible = false; P.cur = null; P.onApply && P.onApply(false, null); };
  P.nastav = async function(id){
    const it = P.list.find(x => x.id === id); if (!it || !P.scene) { P.vypni(); return false; }
    const my = ++P._load; let url;
    if (it.vlastni) { const b = await IDB.get(it.id); if (!b) return false; url = URL.createObjectURL(b); } else url = 'pozadi/' + (it.soubor || it.id + '.jpg');
    return new Promise(res => new THREE.TextureLoader().load(url, t => {
      if (my !== P._load) { t.dispose(); return res(false); }
      t.encoding = P.srgb ? THREE.sRGBEncoding : THREE.LinearEncoding; t.anisotropy = 4;
      if (!P.mesh) { P.mesh = new THREE.Mesh(new THREE.SphereGeometry(P.R, 64, 32), new THREE.MeshBasicMaterial({ side: THREE.BackSide, fog: false, depthWrite: false, depthTest: false }));
        P.mesh.scale.x = -1; P.mesh.renderOrder = -10; P.mesh.frustumCulled = false; P.scene.add(P.mesh); }
      if (P.tex) P.tex.dispose(); P.tex = t; P.mesh.material.map = t; P.mesh.material.needsUpdate = true; P.mesh.visible = true;
      P.mesh.rotation.y = (P.yaw + (it.yaw || 0)) * Math.PI / 180; P.cur = it; it.horizont = horizont(t.image); if (it.slunce === undefined) it.slunce = slunce(t.image);
      P.onApply && P.onApply(true, it); res(true); }, undefined, () => { if (my === P._load) P.vypni(); res(false); }));
  };
  P.natoc = function(deg){ P.yaw = deg; if (P.mesh && P.cur) P.mesh.rotation.y = (P.yaw + (P.cur.yaw || 0)) * Math.PI / 180; };
  P.sleduj = function(pos){ if (P.mesh && P.mesh.visible) P.mesh.position.set(pos.x, 0, pos.z); };   // střed koule u střelce, horizont ve výšce očí
})();
