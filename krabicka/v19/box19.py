"""Skeet Trigger – krabička v19: nosič má v celé tloušťce tvar „horní vrstvy“ – výřezy pod pájkou skrz (ne jen 1 mm),
       zůstávají příčky 45° pod předními rohy, sražené rohy vpravo a drážky u plošek
   v18: XIAO má na okraji půlené plošky (ne kulaté díry) → místo děr Ø2,2 pod každou ploškou
       drážka 1,5 mm otevřená až k hraně desky (drátek připájený z boku na plošku spadne rovnou dolů), mezi drážkami žebra 1 mm
   v17: výřezy pod XIAO vpravo nahoře a dole sražené 45° (žádné ostré vnitřní rohy – kritická místa):
       kapsy pod pájkou u zadní hrany desky a roh průřezu nad vypínačem
   v16: pod oběma předními rohy XIAO příčka 45° (z předního pásu nosiče až pod roh desky),
       díra pinu 1 jí prochází (drátek se protáhne shora), kapsa pod pájkou pinů 1–2 zůstává
   v15: nosič zesílený (2,0 mm místo 1,2 – vzato z rezervy víčka, vnější výška stejná),
       kapsy pod pájenými piny jen shora 1,0 mm hluboké (spodek nosiče zůstává celý, díry pinů dál skrz),
       nad vypínačem nosič úplně vybraný až k okraji, širší rámeček pro desku (vůle 0,4 mm), zesílené rohy u předních šroubů
   v14: 6 magnetů (po 3 vpředu a vzadu), dno pod nimi 0,2 mm
   v13: dno pod magnety 0,2 mm (jedna vrstva)
   v12: jako v11, dno pod magnety jen 0,4 mm (silnější přídrž)
   v11: magnety se vkládají zevnitř (dole plné dno 0,6 mm), v nosiči vstup 2 mm z oválného otvoru ven
   v10: jako v9 + v nosiči průstup 1 mm z děr pinů do výřezů pro přední šrouby (drátky se zasunou z boku)
   v9: jako v8 + víčko o 2,5 mm vyšší, v nosiči díry pod všemi piny XIAO (připájené drátky projdou) a pod ploškami baterky, výřez na drátky u pieza
   v8 (šířka 33, kulatější rohy, šrouby zespodu, bez pera a drážky, čistý otvor pro piezo)
   původně v2 (XIAO nRF52840 Sense + LiPo 38×21, 4 magnety 5×5×5, 4× M2×6)
Souřadnice: x = délka (x=0 vnitřní čelo u USB-C), y = šířka (střed 0), z = výška (z=0 spodek krabičky).
Tisk Prusa MK3S, PETG/PLA černá, 0,2 mm:
  tělo   – dnem dolů, bez podpor (dno sražené 0,5 mm proti „sloní noze“)
  víčko  – VRCHEM NAHORU (stojí na hraně sukně), stropy jsou mosty přes šířku 29 mm, rampy 45° → bez podpor
  nosič  – plochou stranou dolů
"""
import manifold3d as m3, numpy as np, trimesh, math
M, CS = m3.Manifold, m3.CrossSection
m3.set_circular_segments(64)

def box(x0, x1, y0, y1, z0, z1): return M.cube([x1-x0, y1-y0, z1-z0]).translate([x0, y0, z0])
def cyl(x, y, z0, h, d, n=48): return M.cylinder(h, d/2, d/2, n).translate([x, y, z0])
def rrect(x0, x1, y0, y1, r):   # zaoblený obdélník (2D)
    r = max(0.01, min(r, (x1-x0)/2 - 0.01, (y1-y0)/2 - 0.01))
    return CS.square([x1-x0-2*r, y1-y0-2*r]).translate([x0+r, y0+r]).offset(r, m3.JoinType.Round, 2, 64)
def ext(cs, z0, z1): return M.extrude(cs, z1-z0).translate([0, 0, z0])
def prism_xz(pts, y0=-40, y1=40):   # mnohoúhelník v rovině xz vytažený podél y
    ar = sum(pts[i][0]*pts[(i+1)%len(pts)][1] - pts[(i+1)%len(pts)][0]*pts[i][1] for i in range(len(pts)))
    if ar < 0: pts = pts[::-1]
    cs = CS([[(x, z) for x, z in pts]])
    return M.extrude(cs, y1-y0).rotate([90, 0, 0]).translate([0, y1, 0])

# ---------------- rozměry ----------------
WALL = 1.8; R_OUT = 7.0
IW = 29.4                       # vnitřní šířka (deska 17,8 + rámeček + šrouby v rozích)
EB = 6.0                        # čelní / zadní plný blok s magnety
BAY_L, BAY_W = 40.5, 22.4       # šachta baterky 38×21×4 + 2,5 mm vpředu na ohyb drátků
IL = BAY_L + 2*EB               # 51
FLOOR = 1.2; BAY_H = 5.0        # baterka 4 mm + 1 mm na drátky nad ní
RIM = FLOOR + BAY_H             # 8,2 – horní hrana šachty, na ní leží nosič
CAR_T = 2.0                     # v15: nosič 2,0 mm (v14 1,2 – ulomil se vlevo vpředu)
TAPE = 0.0                      # bez pásky – deska leží na nosiči, drží ji rámeček a převis zadní opěry
PCB_L, PCB_W, PCB_T = 21.0, 17.8, 1.2
PCB_X0 = 0.3
PCB_Z0 = RIM + CAR_T + TAPE     # 10,4 spodek desky
PCB_Z1 = PCB_Z0 + PCB_T         # 11,6 vršek desky
USB_H = 3.4                     # změřeno: USB-C 13 mm a čip 11 mm od dna při desce 9,6 mm
CHIP_H = 1.4
SPLIT = RIM + 1.8               # dělicí rovina tělo / víčko
TONGUE_H, TONGUE_T, CLR = 1.4, 0.8, 0.15
LT = 1.6                        # tloušťka víčka
# stropy (vnitřní) – vysoko jen nad USB-C, nad zbytkem desky níž, za deskou nejníž (opírá zarážku)
C1 = PCB_Z1 + USB_H + 0.5       # 15,3   x < X1
C2 = PCB_Z1 + CHIP_H + 0.5      #        X1..X2
C3 = PCB_Z1 + 1.2               # zbytek (nad zadní opěrou)
X1, X2 = 8.0, 20.6
FIL = 2.0                       # zaoblení horních hran víčka
# šrouby v rozích
SC_IN = 2.9; BOSS_D = 5.2; COL_D = 5.2   # sloupky ve víčku silnější
SCREWS = [(SC_IN, s*(IW/2-SC_IN)) for s in (-1, 1)] + [(IL-SC_IN, s*(IW/2-SC_IN)) for s in (-1, 1)]
SEAT_F = SPLIT + 4.0            # vpředu pod hlavou 4 mm plastu (mělké zahloubení)
# vzadu bez zahloubení – hlava sedí na víčku (vyčnívá), pod ní celá výška sloupku
# šrouby: 4× M2×8 (vpředu jde 4 mm do těla, vzadu 4,6 mm)

outer2d = rrect(-WALL, IL+WALL, -IW/2-WALL, IW/2+WALL, R_OUT)
inner2d = rrect(0, IL, -IW/2, IW/2, R_OUT-WALL)
tongue_out2d = rrect(-TONGUE_T, IL+TONGUE_T, -IW/2-TONGUE_T, IW/2+TONGUE_T, R_OUT-WALL+TONGUE_T)
groove2d = rrect(-TONGUE_T-CLR, IL+TONGUE_T+CLR, -IW/2-TONGUE_T-CLR, IW/2+TONGUE_T+CLR, R_OUT-WALL+TONGUE_T+CLR)

# ================= TĚLO =================
body = ext(outer2d, 0, SPLIT)
# sražení spodní hrany 0,5 mm
body = body - (ext(outer2d, -1, 0.5) - M.batch_hull([ext(outer2d.offset(-0.5, m3.JoinType.Round), 0, 0.01), ext(outer2d, 0.5, 0.51)]))
body = body - ext(inner2d, RIM, SPLIT+1)                                      # horní prostor (nosič)
body = body - box(EB, EB+BAY_L, -BAY_W/2, BAY_W/2, FLOOR, RIM+1)             # šachta baterky
for (sx, sy) in SCREWS:
    body = body + (cyl(sx, sy, 0, SPLIT, BOSS_D) ^ ext(inner2d.offset(0.01, m3.JoinType.Round), 0, SPLIT))
    body = body - cyl(sx, sy, -1, SPLIT+2, 2.4, 24) - cyl(sx, sy, -1, 1+2.2, 4.2, 32)   # v8: šroub M2×12 zespodu, hlava zapuštěná 2,2 mm ve dně
MG = 5.15   # v11: těsnější – magnet se zevnitř zatlačí a drží
for x in (EB/2, IL-EB/2):
    for y in (-6.5, 0.0, 6.5):   # v14: 6 magnetů – po třech na čele a vzadu
        body = body - box(x-MG/2, x+MG/2, y-MG/2, y+MG/2, 0.2, RIM+1)          # v13: kapsy na magnety ZEVNITŘ (shora), dole jen jedna vrstva 0,2 mm
body = body - box(IL-3.5, IL+0.01, -4, 4, RIM-1.6, RIM+1)                    # zářez na vypáčení nosiče
# vypínač (posuvný, tělo ~10,4 × 4,5 × 5 mm, páčka 2,8 × 3,6 mm, vyčnívá ~4 mm) v pravém boku vedle baterky
SW_X, SW_L, SW_T = 26.0, 10.9, 4.9          # střed podél x, délka kapsy, výška kapsy (z)
SW_Z = FLOOR + 0.2 + SW_T/2                  # střed výšky
SW_SKIN = 0.7                                # zbylá stěna, kterou prochází páčka
BUMP = 1.4                                   # v7: výstupek na pravém boku – v užší krabičce by se vypínač jinak nevešel
y_out = IW/2 + WALL + BUMP
bump2d = rrect(SW_X - 9.0, SW_X + 9.0, IW/2 + WALL - 1.0, y_out, 1.4)
body = body + M.batch_hull([ext(bump2d.offset(-0.6, m3.JoinType.Round, 2, 32), 0.0, 0.6), ext(bump2d, 0.6, SPLIT - 1.0), ext(bump2d.offset(-0.6, m3.JoinType.Round, 2, 32), SPLIT - 0.4, SPLIT - 0.39)])
body = body - box(SW_X - SW_L/2, SW_X + SW_L/2, BAY_W/2 - 0.5, y_out - SW_SKIN, SW_Z - SW_T/2, RIM + 1)   # kapsa otevřená nahoru – vypínač se vloží shora, nosič ho přidrží
body = body - box(SW_X - 3.0, SW_X + 3.0, y_out - SW_SKIN - 0.5, y_out + 1, SW_Z - 2.0, SW_Z + 2.0)          # průchod páčky (posuv ±1,6 mm)
# ochranné výstupky po stranách páčky (aby se nepřepnul nechtěně), zaoblené
for dx in (-1, 1):
    xg = SW_X + dx * 4.6
    g = M.cylinder(3.0, 1.1, 1.1, 32).rotate([-90, 0, 0]).translate([xg, y_out - 0.5, SW_Z - 1.6]) + \
        M.cylinder(3.0, 1.1, 1.1, 32).rotate([-90, 0, 0]).translate([xg, y_out - 0.5, SW_Z + 1.6]) + \
        box(xg - 1.1, xg + 1.1, y_out - 0.5, y_out + 2.5, SW_Z - 1.6, SW_Z + 1.6)
    body = body + g
# značka zapnuto: tečka vylisovaná u konce dráhy směrem k USB
body = body - M.cylinder(1, 0.7, 0.7, 24).rotate([-90, 0, 0]).translate([SW_X - 4.6, y_out - 0.4, SW_Z + 3.2]) if SW_Z + 3.9 < RIM else body
# USB otvor zasahuje i do pera
USB_Y, USB_W = 0.0, 10.0
USB_Z0, USB_Z1 = PCB_Z1 - 0.4, C1 - 0.1
body = body - box(-WALL-1, 0.5, USB_Y-USB_W/2, USB_Y+USB_W/2, USB_Z0, SPLIT+TONGUE_H+1)

# ================= NOSIČ (mezideska s rámečkem pro XIAO) =================
car = ext(inner2d.offset(-0.25, m3.JoinType.Round), 0, CAR_T)
for (sx, sy) in SCREWS: car = car - cyl(sx, sy, -1, CAR_T + 2, BOSS_D + 0.5)   # v15: těsnější (0,25 mm vůle) – víc materiálu v rozích
for s_ in (-1, 1):   # kapsy pod pájenými kontakty podél krajů po celé délce ohrádky (skrz nosič 1,2 mm), uprostřed nosič plný
    car = car - box(5.6, PCB_X0+PCB_L+0.05, min(s_*4.9, s_*(PCB_W/2+0.05)), max(s_*4.9, s_*(PCB_W/2+0.05)), -1, CAR_T+0.01)   # v15: jen shora 1,0 mm
    # v9: pod KAŽDÝM pinem XIAO (7 na každé straně, rozteč 2,54, řady ±7,62) díra Ø2,2 – drátek připájený do pinu projde nosičem
    for i_ in range(7):
        car = car - box(PCB_X0 + 2.88 + i_*2.54 - 0.75, PCB_X0 + 2.88 + i_*2.54 + 0.75, min(s_*6.9, s_*(PCB_W/2 + 0.45)), max(s_*6.9, s_*(PCB_W/2 + 0.45)), -1, CAR_T + 2)   # v18: drážka k hraně desky
    car = car - box(2.0, 5.7, min(s_*6.6, s_*8.7), max(s_*6.6, s_*8.7), -1, CAR_T+0.01)   # spojit díry pinů 1–2 s kapsou (v15 jen shora)
    # v10: průstup 1 mm z kapsy pinů do výřezu pro šroub (a ten je otevřený ven) – připájené drátky se do děr zasunou z boku
    car = car - box(3.4, 4.4, min(s_*8.0, s_*10.5), max(s_*8.0, s_*10.5), -1, CAR_T+0.01)
    # v15: tady se v14 ulomil – mezi dírou pinu 1, průstupem a výřezem šroubu zůstávaly proužky 0,2–1,8 mm.
    #      Roh před pinem 1 se celý vybere (pin 1 je otevřený dopředu, drátek se zasune zepředu nebo z boku),
    #      takže nikde nezůstane tenký výběžek; zbytek nosiče má kolem děr aspoň 1,5 mm a je 2,0 mm silný.
    car = car - ext(rrect(-2.0, 4.4, min(s_*6.7, s_*16), max(s_*6.7, s_*16), 0.6), -1, CAR_T+0.01)
    # v16: příčka 45° pod rohem XIAO – trojúhelník (0,25; 6,6) – (2,95; 6,6) – (0,25; 9,3), přepona pod 45°
    tri = CS([[(0.25, s_*6.6), (2.95, s_*6.6), (0.25, s_*9.3)]] if s_ > 0 else [[(0.25, -6.6), (0.25, -9.3), (2.95, -6.6)]])
    car = car + ext(tri, 0, CAR_T)
    car = car - box(PCB_X0 + 2.88 + 0*2.54 - 0.75, PCB_X0 + 2.88 + 0*2.54 + 0.75, min(s_*6.9, s_*(PCB_W/2 + 0.45)), max(s_*6.9, s_*(PCB_W/2 + 0.45)), -1, CAR_T + 2)   # drážka pinu 1 znovu skrz
    car = car - box(2.0, 5.7, min(s_*6.6, s_*8.7), max(s_*6.6, s_*8.7), -1, CAR_T+0.01)      # kapsa pod pájkou znovu
    # v9: pod ploškami baterky (BAT+ / BAT− na spodku desky ve výšce pinů D2/D3) rozšířená kapsa ke středu
    car = car - box(6.8, 12.4, min(s_*2.8, s_*5.0), max(s_*2.8, s_*5.0), -1, CAR_T+0.01)
# průřez nad vypínačem: kontakty ohnuté nahoru projdou nosičem (nosič na nich neleží); vnější pás 1,25 mm vypínač dál přidrží
# změřeno: zadní strana kontaktů 7,5 mm od vnější hrany krabičky → y = 16,3 − 7,5 = 8,8; kontakty sahají ven ke kapse
SWC_Y0 = y_out - 7.5 - 0.6                                                     # 0,6 mm vůle za kontakty
car = car - ext(rrect(PCB_X0 + PCB_L + 0.3, SW_X + SW_L/2 - 0.2, SWC_Y0, BAY_W/2 + 0.6, 0.8), -1, CAR_T+0.01)
car = car - ext(rrect(SW_X - SW_L/2 - 0.4, SW_X + SW_L/2 + 0.4, SWC_Y0, IW/2 + 3, 0.8), -1, CAR_T+0.01)   # v15: nad celým vypínačem nosič vybraný až k okraji
car = car - box(EB+BAY_L-3.5, IL+1, 2.5, 7.5, -1, 3)                          # vzadu zářez „U“ 5 mm široký až do kraje – připájený kabel se jen vloží
car = car - box(IL-3.5, IL+1, -4, 4, -1, 3)                                   # zářez na páčení (shodný s tělem)
RAIL_T = 1.4; RH = (C3 - 0.25) - (RIM + CAR_T)                                 # boky rámečku do stejné výšky jako zadní opěra
for s in (-1, 1):
    y0, y1 = sorted([s*(PCB_W/2+0.4), s*(PCB_W/2+0.4+RAIL_T)])   # v15: vůle 0,4 mm (deska s připájenými drátky jde dovnitř volně)
    car = car + box(5.6, PCB_X0+PCB_L+0.4, y0, y1, CAR_T, CAR_T+RH)
    car = car - box(11.5, 15.0, y0-0.1, y1+0.1, CAR_T-0.01, CAR_T+RH+1)       # v8: mezera 3,5 mm v boku ohrádky pro drátky
# zadní opěra: plná stěna do výšky nad desku + 3 žebra; strop víčka na ni dosedne (0,2 mm)
BS_X0 = PCB_X0 + PCB_L + 0.4   # v15: +0,2 mm vůle za deskou
BS_Z1 = C3 - 0.2 - RIM                                                        # vršek opěry (relativně k nosiči)
car = car + box(BS_X0, BS_X0+2.6, -(PCB_W/2+0.4+RAIL_T), PCB_W/2+0.4+RAIL_T, CAR_T, BS_Z1)
for y in (-6.0, -3.0, 4.5, 6.6):   # žebra mimo průchod drátků uprostřed; pravé vnější posunuto dovnitř kvůli kontaktům vypínače
    car = car + prism_xz([(BS_X0+2.0, CAR_T), (BS_X0+7.5, CAR_T), (BS_X0+2.0, BS_Z1-0.4)], y-0.7, y+0.7)
car = car - box(BS_X0 - 0.1, BS_X0 + 2.7, -1.6, 1.6, CAR_T - 0.01, 30)   # v8: průchod drátků uprostřed zadní opěry (k piezu)
car = car - box(BS_X0 - 0.1, BS_X0 + 2.7, SWC_Y0, PCB_W/2 + 0.4 + RAIL_T + 0.1, CAR_T - 0.01, 30)   # zářez v zadní opěře nad kontakty vypínače
# v17: sražení 45° vnitřních rohů výřezů pod XIAO vpravo (u zadní hrany desky), nahoře i dole
KX = PCB_X0 + PCB_L + 0.05                      # zadní konec kapes 21,35
for s_ in (-1, 1):
    def T3(pts): return ext(CS([[(x, s_*y) for x, y in (pts if s_ > 0 else pts[::-1])]]), 0, CAR_T)
    car = car + T3([(KX + 0.01, PCB_W/2 + 0.06), (KX - 1.75, PCB_W/2 + 0.06), (KX + 0.01, PCB_W/2 - 1.7)])   # vnější roh kapsy
    car = car + T3([(KX + 0.01, 4.89), (KX + 0.01, 6.65), (KX - 1.75, 4.89)])                               # vnitřní roh kapsy
# roh průřezu nad vypínačem (u zadní opěry desky) taky 45°
SWX0 = SW_X - SW_L/2 - 0.4
car = car + ext(CS([[(SWX0 - 0.01, SWC_Y0 - 0.01), (SWX0 + 1.6, SWC_Y0 - 0.01), (SWX0 - 0.01, SWC_Y0 + 1.6)]]), 0, CAR_T)
# převis 0,8 mm nad zadní hranou desky: deska se zasune zadní hranou pod něj a vpředu ji drží strop u USB-C


# ================= VÍČKO v7: jako v6 + posuvný vypínač ve výstupku na pravém boku, tlačítko v boční liště (vodorovně), piezo zapuštěné až na baterku =================
# Tlačítko TS6601 6×6, výška 7,0 (tělo 3,5 + hmatník 3,5) leží na boku: hmatník vyčnívá z levé stěny o 0,8 mm (žádný důlek).
# Piezo PT-2040PQ Ø22 × 7 sedí v otvoru nosiče na podložce 0,6 mm přímo na baterce, shora ho přitlačí trubka z víčka.
SW_E = 6.0; SW_BODY = 3.5; SW_H = 7.0; SW_A = 6.3
PZ_D, PZ_H = 22.0, 7.0
PZ_X, PZ_Y = 40.75, 0.0
PAD = 0.3                                         # tenká podložka/izolace pod piezem; vzadu je pro piezo vybrání v těle
PZ_Z0 = FLOOR + 4.0 + PAD                         # spodek pieza 5,8 (baterka 4 mm)
PZ_Z1 = PZ_Z0 + PZ_H                              # vršek pieza 12,8
BT_X = 27.3                                       # tlačítko: mezi deskou a piezem, levý bok
BT_Z = SPLIT + 0.4                                # osa hmatníku v dělicí liště (půl otvoru v těle, půl ve víčku)
CEIL = max(PZ_Z1 + 0.2, BT_Z + SW_E/2 + 0.1, C1 + 0.2) + 1.7   # v15: rezerva 2,5 → 1,7 (0,8 mm si vzal silnější nosič), vnější výška beze změny (místo na drátky nad deskou)
LT = 1.5
T = CEIL + LT
y_wall = -(IW/2 + WALL)                           # vnější plocha levého boku
SW_Y_TIP = y_wall - 0.8                           # špička hmatníku 0,8 mm venku
SW_Y_BASE = SW_Y_TIP + SW_H                       # zadní plocha těla (směrem dovnitř)
SW_Y_BODY = SW_Y_BASE - SW_BODY                   # přední plocha těla
CH = 1.2
def chamfered(h, cs=outer2d, c=CH):
    return M.batch_hull([ext(cs, SPLIT, h - c), ext(cs.offset(-c, m3.JoinType.Round, 2, 64), h - 0.01, h)])
lid = chamfered(T) - ext(inner2d, SPLIT-1, CEIL)
for (sx, sy) in SCREWS:
    col = (cyl(sx, sy, SPLIT, CEIL-SPLIT+0.1, COL_D) ^ ext(inner2d.offset(1.0, m3.JoinType.Round), SPLIT, 40))
    lid = lid + col
for (sx, sy) in SCREWS:
    lid = lid - cyl(sx, sy, SPLIT-1, 1+4.6, 1.65, 24)          # v8: slepá díra pro samořezný M2 zespodu (4,6 mm), vršek víčka v rozích bez děr
lid = lid + (box(0.6, 7.0, -4.0, 4.0, PCB_Z1 + USB_H + 0.3, CEIL + 0.1))
lid = lid + (box(PCB_X0 + PCB_L + 0.3, PCB_X0 + PCB_L + 2.5, -7.5, -5.0, C3 - 0.2, CEIL + 0.1))
lid = lid + (box(PCB_X0 + PCB_L + 0.3, PCB_X0 + PCB_L + 2.5, 5.0, 7.5, C3 - 0.2, CEIL + 0.1))
def rbox_yz(x0, x1, yc, w, z0, z1, r):
    b_ = box(x0, x1, yc-w/2+r, yc+w/2-r, z0, z1) + box(x0, x1, yc-w/2, yc+w/2, z0+r, z1-r)
    for yy in (yc-w/2+r, yc+w/2-r):
        for zz in (z0+r, z1-r):
            b_ = b_ + M.cylinder(x1-x0, r, r, 32).rotate([0, 90, 0]).translate([x0, yy, zz])
    return b_
lid = lid - rbox_yz(-WALL-1, 0.5, USB_Y, USB_W, USB_Z0, USB_Z1, 1.0)
lid = lid - rbox_yz(-WALL-1, -WALL+1.0, USB_Y, 12.0, PCB_Z1-0.6, PCB_Z1+5.2, 2.0)
for s in (-1, 1):
    lid = lid + cyl(2.6, s*6.4, PCB_Z1 + 0.7, CEIL - PCB_Z1, 3.4, 32)
    lid = lid - cyl(2.6, s*6.4, PCB_Z1, 40, 1.9, 24)
for i in range(3):
    for j in range(3): lid = lid - cyl(11 + i*2.8, -2.8 + j*2.8, SPLIT, 40, 1.4, 20)
# ---- boční lišta: mělká drážka po obou bocích ve výšce osy tlačítka; tlačítko z ní vyčnívá ----
LISTA_H = 1.2; LISTA_D = 0.5                      # obvodová lišta (drážka) v dělicí rovině – opticky snižuje krabičku
lista_cut = ext(outer2d, SPLIT - LISTA_H/2, SPLIT + LISTA_H/2) - ext(outer2d.offset(-LISTA_D, m3.JoinType.Round, 2, 64), SPLIT - LISTA_H, SPLIT + LISTA_H)
lid = lid - lista_cut
# ---- tlačítko vodorovně: otvor v boku, kolébka visí ze stropu, opěra za tělem mezi nožičkami ----
lid = lid - M.cylinder(10, 2.25, 2.25, 48).rotate([90, 0, 0]).translate([BT_X, y_wall + 5, BT_Z])          # v8: otvor Ø4,5 pro hmatník Ø3,5
lid = lid - M.cylinder(0.6, 2.25, 2.85, 48).rotate([90, 0, 0]).translate([BT_X, y_wall + 0.6, BT_Z])       # sražení hrany otvoru zvenku (kužel 0,6 mm)
wa = SW_A/2
crad = box(BT_X - wa - 1.2, BT_X + wa + 1.2, -IW/2 - 0.01, SW_Y_BASE + 1.2, max(BT_Z - SW_E/2 + 1.0, C3 + 0.1), CEIL + 0.1)
crad = crad - box(BT_X - wa, BT_X + wa, -IW/2 - 1, SW_Y_BASE, BT_Z - SW_E/2 - 1, BT_Z + SW_A/2)            # prostor pro tělo spínače (zespodu otevřený)
crad = crad - box(BT_X - wa - 0.6, BT_X + wa + 0.6, SW_Y_BASE - 0.01, SW_Y_BASE + 2, BT_Z - SW_E/2 - 1, BT_Z + 2.6)   # nožičky za tělem
crad = crad + box(BT_X - 1.6, BT_X + 1.6, SW_Y_BASE, SW_Y_BASE + 1.2, max(BT_Z - 1.5, SPLIT + 0.2), CEIL + 0.1)             # opěra mezi nožičkami
for dx_ in (-1, 1): crad = crad + box(*sorted([BT_X + dx_*(wa + 0.7), BT_X + dx_*(wa + 2.2)]), SW_Y_BASE - 2.0, SW_Y_BASE + 1.6, C3 + 0.1, CEIL + 0.1)   # v8: opěry i vně nožiček (tlačítko se nevyviklá dozadu)
lid = lid + crad
# ---- v8 piezo: čistý kruhový otvor Ø22,6 skrz víčko (piezo Ø22 je vidět), trubka kolem a 3 malé zuby přitlačí okraj pieza ----
PZ_HOLE = 22.6
lid = lid + (cyl(PZ_X, PZ_Y, PZ_Z1 + 0.15, CEIL - PZ_Z1, PZ_HOLE + 2.4, 96) - cyl(PZ_X, PZ_Y, PZ_Z1, 10, PZ_HOLE, 96))
lid = lid - cyl(PZ_X, PZ_Y, CEIL - 1, 10, PZ_HOLE, 96)
lid = lid - M.batch_hull([cyl(PZ_X, PZ_Y, T - 0.6, 0.01, PZ_HOLE, 96), cyl(PZ_X, PZ_Y, T, 0.01, PZ_HOLE + 1.2, 96)])   # sražení hrany otvoru
for k in range(3):
    a = math.radians(90 + k*120); cx, cy = PZ_X + (PZ_HOLE/2 - 0.4)*math.cos(a), PZ_Y + (PZ_HOLE/2 - 0.4)*math.sin(a)
    lid = lid + (cyl(cx, cy, PZ_Z1 + 0.15, 1.6, 1.8, 24) ^ cyl(PZ_X, PZ_Y, PZ_Z1, 3, PZ_HOLE + 0.5, 96))
# ---- tělo: v místě tlačítka vyříznout pero (hmatník prochází stěnou) ----
# ---- nosič: otvor pro piezo (sedí na baterce) a podložka pod tlačítko ----
car = car - cyl(PZ_X, PZ_Y, -1, 5, PZ_D + 0.8, 96)
# v9: výřez na drátky pieza na okraji otvoru – 3 mm podél okraje, 2 mm do hloubky (směrem k desce i dozadu)
car = car - box(PZ_X - PZ_D/2 - 0.4 - 2.0, PZ_X - PZ_D/2, -1.5, 1.5, -1, 30)
car = car - box(PZ_X + PZ_D/2, PZ_X + PZ_D/2 + 0.4 + 2.0, -1.5, 1.5, -1, 30)
car = car - box(BT_X - SW_A/2 - 0.7, BT_X + SW_A/2 + 0.7, -IW/2 - 1, SW_Y_BASE + 1.8, -1, 5)
car = car - box(BT_X - 3.6, BT_X - 2.9, -IW/2, -6.3, -1, 30)   # v8: zářez pro nožičky tlačítka
# ---- tělo: kapsa pro spínač (leží na okraji šachty nad baterkou), půl otvoru pro hmatník, lišta, vybrání pro piezo ----
SW_Z0 = BT_Z - SW_E/2
body = body - box(BT_X - SW_A/2 - 0.45, BT_X + SW_A/2 + 0.45, -IW/2 - 0.01, SW_Y_BASE + 0.45, SW_Z0 - 0.35, SPLIT + 3)
body = body - M.cylinder(10, 2.25, 2.25, 48).rotate([90, 0, 0]).translate([BT_X, y_wall + 5, BT_Z])
body = body - M.cylinder(0.6, 2.25, 2.85, 48).rotate([90, 0, 0]).translate([BT_X, y_wall + 0.6, BT_Z])
body = body - lista_cut
body = body - cyl(PZ_X, PZ_Y, PZ_Z0 - 0.05, 3, PZ_D + 0.6, 96)
# ================= export =================
def tm(p):
    mm = p.to_mesh(); return trimesh.Trimesh(np.array(mm.vert_properties)[:, :3], np.array(mm.tri_verts))
def save(p, name, flip=False):
    mm_ = p.to_mesh(); t = trimesh.Trimesh(np.array(mm_.vert_properties)[:, :3], np.array(mm_.tri_verts), process=False)
    if flip: t.apply_transform(trimesh.transformations.rotation_matrix(np.pi, [1, 0, 0]))
    t.apply_translation([0, 0, -t.bounds[0][2]]); t.export(name)
    print(f'{name:28s} {"vodotěsný" if t.is_watertight else "!!! NE"}  {np.round(t.extents, 1)}  {t.volume/1000:.1f} cm³')
bb=lid.bounding_box(); print('bbox',[round(v,1) for v in bb])
save(lid, 'krabicka19_vicko.stl', flip=True)   # tiskne se VRCHEM DOLŮ (vršek na podložce) – žádné mosty pod mřížkou
save(car, 'krabicka19_nosic.stl')
save(body, 'krabicka19_telo.stl')
print('vnější rozměr', IL+2*WALL, 'x', IW+2*WALL, 'x', round(T, 1), ' strop', round(CEIL,1))

# kontrola kolizí
SWM = box(BT_X-3,BT_X+3,SW_Y_BODY,SW_Y_BASE,BT_Z-3,BT_Z+3) + M.cylinder(SW_H-SW_BODY,1.75,1.75,32).rotate([90,0,0]).translate([BT_X,SW_Y_BODY,BT_Z]) + box(BT_X-3.35,BT_X-3.1,SW_Y_BASE-0.01,SW_Y_BASE+3.5,BT_Z-2.6,BT_Z-1.9) + box(BT_X+3.1,BT_X+3.35,SW_Y_BASE-0.01,SW_Y_BASE+3.5,BT_Z+1.9,BT_Z+2.6)
PZM = cyl(PZ_X,PZ_Y,PZ_Z0,PZ_H,PZ_D)
pcb = box(PCB_X0, PCB_X0+PCB_L, -PCB_W/2, PCB_W/2, PCB_Z0, PCB_Z1) + box(PCB_X0-0.7, PCB_X0+7.3, -4.5, 4.5, PCB_Z1, PCB_Z1+USB_H) + box(PCB_X0+8, PCB_X0+PCB_L-1.5, -7, 7, PCB_Z1, PCB_Z1+CHIP_H)
bat = box(EB+2.0, EB+40.0, -10.5, 10.5, FLOOR, FLOOR+4.0)
carA = car.translate([0, 0, RIM])
for nm, a, b in [('deska×víčko', pcb, lid), ('deska×tělo', pcb, body), ('deska×nosič', pcb, carA), ('baterka×tělo', bat, body), ('nosič×tělo', carA, body), ('nosič×víčko', carA, lid), ('tělo×víčko', body, lid), ('piezo×nosič', PZM, carA), ('piezo×víčko', PZM, lid), ('piezo×tělo', PZM, body), ('piezo×baterka', PZM, bat), ('spínač×deska', SWM, pcb), ('spínač×víčko', SWM, lid), ('spínač×nosič', SWM, carA), ('spínač×tělo', SWM, body)]:
    v=(a ^ b); print(f'kolize {nm:14s} {v.volume():.3f} mm³', [round(x,2) for x in v.bounding_box()] if v.volume()>0.01 else '')
np.save('_parts.npy', np.array([0]))
import pickle
pickle.dump({'sw':(np.array(SWM.to_mesh().vert_properties)[:,:3],np.array(SWM.to_mesh().tri_verts)),'pz':(np.array(PZM.to_mesh().vert_properties)[:,:3],np.array(PZM.to_mesh().tri_verts)),**{k: (np.array(v.to_mesh().vert_properties)[:, :3], np.array(v.to_mesh().tri_verts)) for k, v in
             {'body': body, 'car': carA, 'lid': lid, 'pcb': pcb, 'bat': bat}.items()}}, open('_parts19.pkl', 'wb'))
