# Hlášky z vlastní nahrávky (Tomáš, Diktafon) → 8 kHz / 8 bit pro piezo, stejné pořadí a jména jako gen.py
import numpy as np, scipy.signal as sg, scipy.io.wavfile as wf, re, sys
H=re.findall(r"\('(\w+)','([^']*)'\)", open(sys.argv[1]).read())
r,x=wf.read('rec.wav'); x=x.astype(np.float32)/32768
seg=[(0.3,1.3),(2.74,0.22),(4.06,0.18),(5.34,0.6),(6.72,0.58),(8.3,0.9),(10.14,1.54),(12.54,0.96),(14.14,1.02),(15.88,1.0),(17.0,0.55),(18.38,0.78),(19.84,0.5),(21.4,0.94),(23.26,0.72),(24.94,0.82),(26.46,0.78),(28.0,0.74),(29.38,0.8),(30.82,0.38),(32.38,0.68),(34.02,0.48),(36.56,0.34),(38.06,1.38),(40.12,1.2),(42.24,0.64),(43.8,0.5),(45.3,0.48),(46.8,0.78),(50.2,0.56),(52.8,0.46),(54.18,0.48),(55.36,0.48),(56.72,0.34),(57.84,0.52),(59.12,0.3),(60.14,0.44),(61.32,0.5),(62.56,0.46),(63.56,0.42)]
assert len(seg)==len(H)==40
SR=8000; out=[]
for (k,t),(a,d) in zip(H,seg):
    y=x[int((a-0.06)*r):int((a+d+0.12)*r)]
    # přesný ořez ticha
    e=np.abs(y)>0.04*np.abs(y).max(); i0=max(0,np.argmax(e)-int(0.015*r)); i1=len(y)-np.argmax(e[::-1])+int(0.04*r); y=y[i0:i1]
    y=sg.resample_poly(y,SR,r)
    b,a_=sg.butter(2,350/(SR/2),'high'); y=sg.lfilter(b,a_,y)
    b,a_=sg.butter(2,[1500/(SR/2),3600/(SR/2)],'band'); y=y+1.2*sg.lfilter(b,a_,y)
    y=y/np.abs(y).max()
    # brána proti šumu v pauzách, pak komprese a ořez (hlasitější z pieza)
    env=np.convolve(np.abs(y),np.ones(80)/80,'same'); y=y*np.clip((env-0.02)/0.04,0,1)
    y=np.sign(y)*np.abs(y)**0.35; y=np.clip(y*1.5,-1,1)
    n=int(0.01*SR); w=np.ones(len(y)); w[:n]=np.linspace(0,1,n); w[-n:]=np.linspace(1,0,n); y*=w
    u=np.clip(np.round(128+y*127),0,255).astype(np.uint8); out.append((k,t,u)); wf.write(f'pz_{k}.wav',SR,u)
with open('hlasky.h','w') as f:
    f.write('// Hlášky pro piezo: 8 kHz, 8 bit bez znaménka (128 = ticho). Vlastní hlas (nahrávka z Diktafonu), zpracováno hlasky_vlastni.py.\n#pragma once\n#include <stdint.h>\n')
    for k,t,u in out: f.write(f'// "{t}"\nconst uint8_t HL_{k}[{len(u)}] = {{'+','.join(map(str,u))+'};\n')
    f.write('struct Hlaska { const char* jm; const uint8_t* d; uint32_t n; };\nconst Hlaska HLASKY[] = {\n'+''.join(f'  {{"{k}", HL_{k}, {len(u)}}},\n' for k,t,u in out)+'};\nconst uint8_t HLASEK = sizeof(HLASKY)/sizeof(HLASKY[0]);\n')
    f.write('enum { '+', '.join(f'H_{k.upper()}' for k,t,u in out)+' };\n')
tot=sum(len(u) for _,_,u in out); print('celkem %.1f s, %d kB'%(tot/SR,tot//1024))
import wave
o=wave.open('ukazka_vlastni.wav','wb'); o.setnchannels(1); o.setsampwidth(1); o.setframerate(SR)
for k,t,u in out: o.writeframes(u.tobytes()+b'\x80'*int(0.35*SR))
o.close()
