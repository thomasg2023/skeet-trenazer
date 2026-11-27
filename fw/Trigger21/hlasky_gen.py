import subprocess, numpy as np, scipy.signal as sg, scipy.io.wavfile as wf, os
H=[('zapnuto','Spoušť zapnuta'),('start','Start'),('stop','Stop'),('pripojeno','Připojeno'),('odpojeno','Odpojeno'),
   ('slaba','Slabá baterie'),('vybito','Baterie vybitá, vypínám'),('bat_plna','Baterie plná'),('bat_pul','Baterie polovina'),('bat_malo','Baterie skoro prázdná'),
   ('dobre','Dobře'),('zastavil','Zastavil jsi'),('zpomalujes','Zpomaluješ'),('dotahni','Dotáhni švih'),('trhany','Trhaný švih'),
   ('pamet_plna','Paměť plná'),('chyba_senzoru','Chyba senzoru'),('vypinam','Vypínám'),('nabiji','Nabíjím')]
SR=8000; out=[]; tot=0
for k,t in H:
    subprocess.run(['espeak-ng','-v','mb-cz2','-s','165','-a','180','-w',f'{k}.wav',t],check=True,capture_output=True)
    r,x=wf.read(f'{k}.wav'); x=x.astype(np.float32); x/=max(1,np.abs(x).max())
    # ořez ticha
    e=np.abs(x)>0.02; i0=max(0,np.argmax(e)-int(0.01*r)); i1=len(x)-np.argmax(e[::-1])+int(0.03*r); x=x[i0:i1]
    y=sg.resample_poly(x,SR,r)
    # piezo: horní propust 350 Hz, zvýraznění 1,5–3,5 kHz, komprese, normalizace
    b,a=sg.butter(2,350/(SR/2),'high'); y=sg.lfilter(b,a,y)
    b,a=sg.butter(2,[1500/(SR/2),3600/(SR/2)],'band'); y=y+1.2*sg.lfilter(b,a,y)
    y=y/np.abs(y).max(); y=np.sign(y)*np.abs(y)**0.6   # komprese dynamiky (hlasitější tiché hlásky)
    y=np.clip(y*1.15,-1,1)
    u=np.clip(np.round(128+y*127),0,255).astype(np.uint8)
    out.append((k,t,u)); tot+=len(u)
    wf.write(f'pz_{k}.wav',SR,u)
with open('hlasky.h','w') as f:
    f.write('// Hlášky pro piezo: 8 kHz, 8 bit bez znaménka (128 = ticho). Vygenerováno espeak-ng + MBROLA cz2.\n#pragma once\n#include <stdint.h>\n')
    for k,t,u in out:
        f.write(f'// "{t}"\nconst uint8_t HL_{k}[{len(u)}] = {{'+','.join(map(str,u))+'};\n')
    f.write('struct Hlaska { const char* jm; const uint8_t* d; uint32_t n; };\nconst Hlaska HLASKY[] = {\n'+''.join(f'  {{"{k}", HL_{k}, {len(u)}}},\n' for k,t,u in out)+'};\nconst uint8_t HLASEK = sizeof(HLASKY)/sizeof(HLASKY[0]);\n')
    f.write('enum { '+', '.join(f'H_{k.upper()}' for k,t,u in out)+' };\n')
print('celkem vzorků',tot,'= %.1f s, %d kB'%(tot/SR,tot//1024))
