from pathlib import Path
from uuid import uuid4
import heapq, math, os, re
import pcbnew

ROOT = Path(__file__).resolve().parents[1]
LIBDIR = ROOT / "QUARK.pretty"
def find_footprints_dir():
    configured = os.environ.get("KICAD_FOOTPRINTS_DIR")
    if configured:
        candidate = Path(configured)
        if candidate.is_dir(): return candidate
        raise RuntimeError(f"KICAD_FOOTPRINTS_DIR does not exist: {candidate}")
    candidates = [Path("/usr/share/kicad/footprints"), Path("/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints")]
    candidates += sorted(Path(r"C:\Program Files\KiCad").glob("*/share/kicad/footprints"), reverse=True)
    for candidate in candidates:
        if candidate.is_dir(): return candidate
    raise RuntimeError("KiCad footprints not found; set KICAD_FOOTPRINTS_DIR to the KiCad footprints folder")

FOOTPRINTS_DIR = find_footprints_dir()
BOARD_PATH = ROOT / "QUARK_SuperMini_Carrier.kicad_pcb"
SCALE = 1_000_000
STEP_MM = 0.254
TRACE_MM = 0.254
CLEARANCE_MM = 0.20
VIA_MM = 0.60
DRILL_MM = 0.30
W_MM, H_MM = 70.104, 55.372
LAYER_F, LAYER_B = pcbnew.F_Cu, pcbnew.B_Cu

def mm(v): return pcbnew.FromMM(v)
def vec(x,y): return pcbnew.VECTOR2I(mm(x),mm(y))
def uid(): return str(uuid4())

def make_socket_footprint():
    LIBDIR.mkdir(parents=True, exist_ok=True)
    parts=['''(footprint "ESP32C3_SuperMini_Socket_2x8"
 (version 20241229)
 (generator "pcbnew")
 (layer "F.Cu")
 (descr "2x8 female socket footprint for ESP32-C3 SuperMini; 15.24mm row spacing; USB-C end at top")
 (attr through_hole)
 (fp_text reference "U1" (at 0 -13.2) (layer "F.SilkS") (effects (font (size 1 1) (thickness 0.15))))
 (fp_text value "ESP32-C3 SuperMini" (at 0 13.2) (layer "F.Fab") (effects (font (size 1 1) (thickness 0.15))))
 (fp_rect (start -9 -11.26) (end 9 11.26) (stroke (width 0.10) (type default)) (fill none) (layer "F.Fab") (uuid "'''+uid()+'''"))
 (fp_line (start -5 -11.26) (end 5 -11.26) (stroke (width 0.25) (type default)) (layer "F.Fab") (uuid "'''+uid()+'''"))
 (fp_text user "USB-C" (at 0 -10.0) (layer "F.Fab") (effects (font (size 0.9 0.9) (thickness 0.12))))
 (fp_text user "ANTENNA" (at 0 9.4) (layer "F.Fab") (effects (font (size 0.8 0.8) (thickness 0.12))))
''']
    left=["5V","GND","3V3","GPIO4","GPIO3","GPIO2","GPIO1","GPIO0"]
    right=["GPIO5","GPIO6","GPIO7","GPIO8","GPIO9","GPIO10","GPIO20","GPIO21"]
    for i,name in enumerate(left):
        y=-8.89+i*2.54
        parts.append(f' (pad "{i+1}" thru_hole {"rect" if i==0 else "circle"} (at -7.62 {y:.3f}) (size 1.7 1.7) (drill 1.0) (layers "*.Cu" "*.Mask") (uuid "{uid()}"))\n')
        parts.append(f' (fp_text user "{name}" (at -10.1 {y:.3f} 0) (layer "F.SilkS") (effects (font (size 0.82 0.82) (thickness 0.12)) (justify right)))\n')
    for i,name in enumerate(right):
        y=-8.89+i*2.54
        parts.append(f' (pad "{i+9}" thru_hole circle (at 7.62 {y:.3f}) (size 1.7 1.7) (drill 1.0) (layers "*.Cu" "*.Mask") (uuid "{uid()}"))\n')
        parts.append(f' (fp_text user "{name}" (at 10.1 {y:.3f} 0) (layer "F.SilkS") (effects (font (size 0.82 0.82) (thickness 0.12)) (justify left)))\n')
    parts.append(" (embedded_fonts no)\n)\n")
    (LIBDIR/"ESP32C3_SuperMini_Socket_2x8.kicad_mod").write_text("".join(parts),encoding="utf-8")
    table='(fp_lib_table\n (lib (name "QUARK") (type "KiCad") (uri "'+chr(36)+'{KIPRJMOD}/QUARK.pretty") (options "") (descr "QUARK project footprints"))\n)\n'
    (ROOT/"fp-lib-table").write_text(table,encoding="utf-8")

def load_footprint(lib,name):
    item=pcbnew.FootprintLoad(str(FOOTPRINTS_DIR/(lib+".pretty")),name)
    if item is None: raise RuntimeError(f"Cannot load {lib}:{name}")
    return item

def add_fp(board, ref, value, lib, name, x,y, nets=None, ref_pos=None):
    fp=load_footprint(lib,name); fp.SetFPID(pcbnew.LIB_ID(lib,name)); fp.SetReference(ref); fp.SetValue(value); fp.SetPosition(vec(x,y))
    if ref in symbol_uuids: fp.SetPath(pcbnew.KIID_PATH("/"+symbol_uuids[ref]))
    fp.Reference().SetPosition(vec(*(ref_pos or (x,y-2.2))))
    fp.Reference().SetLayer(pcbnew.F_SilkS)
    fp.Reference().SetTextSize(vec(1.0,1.0)); fp.Reference().SetTextThickness(mm(0.15))
    fp.Value().SetVisible(False)
    if nets:
        for pad in fp.Pads():
            net=nets.get(str(pad.GetNumber()))
            if net: pad.SetNet(netdict[net])
    board.Add(fp); footprints[ref]=fp
    return fp

def add_local_fp(board,ref,value,name,x,y,nets=None,ref_pos=None):
    fp=pcbnew.FootprintLoad(str(LIBDIR),name)
    if fp is None: raise RuntimeError(f"Cannot load QUARK:{name}")
    fp.SetFPID(pcbnew.LIB_ID("QUARK",name)); fp.SetReference(ref); fp.SetValue(value); fp.SetPosition(vec(x,y))
    if ref in symbol_uuids: fp.SetPath(pcbnew.KIID_PATH("/"+symbol_uuids[ref]))
    fp.Reference().SetPosition(vec(*(ref_pos or (x,y-2.5))))
    fp.Reference().SetLayer(pcbnew.F_SilkS)
    fp.Reference().SetTextSize(vec(1.0,1.0)); fp.Reference().SetTextThickness(mm(0.15))
    fp.Value().SetVisible(False)
    if nets:
        for pad in fp.Pads():
            net=nets.get(str(pad.GetNumber()))
            if net: pad.SetNet(netdict[net])
    board.Add(fp); footprints[ref]=fp
    return fp

def add_text(board,text,x,y,size=1.0,layer=pcbnew.F_SilkS,justify="center"):
    t=pcbnew.PCB_TEXT(board); t.SetText(text); t.SetPosition(vec(x,y)); t.SetLayer(layer)
    t.SetTextSize(vec(size,size)); t.SetTextThickness(mm(max(0.12,size*0.13)))
    if justify=="left": t.SetHorizJustify(pcbnew.GR_TEXT_H_ALIGN_LEFT)
    elif justify=="right": t.SetHorizJustify(pcbnew.GR_TEXT_H_ALIGN_RIGHT)
    else: t.SetHorizJustify(pcbnew.GR_TEXT_H_ALIGN_CENTER)
    t.SetVertJustify(pcbnew.GR_TEXT_V_ALIGN_CENTER); board.Add(t)

def add_edge_line(board,x1,y1,x2,y2):
    s=pcbnew.PCB_SHAPE(board); s.SetShape(pcbnew.SHAPE_T_SEGMENT)
    s.SetStart(vec(x1,y1)); s.SetEnd(vec(x2,y2)); s.SetLayer(pcbnew.Edge_Cuts); s.SetWidth(mm(0.05)); board.Add(s)

def add_outline(board):
    pts=[(0.0,0.0),(W_MM,0.0),(W_MM,H_MM),(0.0,H_MM),(0.0,0.0)]
    for a,b in zip(pts,pts[1:]): add_edge_line(board,*a,*b)

def add_keepout(board):
    z=pcbnew.ZONE(board); z.SetIsRuleArea(True)
    layers=pcbnew.LSET(); layers.AddLayer(pcbnew.F_Cu); layers.AddLayer(pcbnew.B_Cu)
    z.SetLayerSet(layers); z.SetDoNotAllowTracks(True); z.SetDoNotAllowVias(True)
    z.SetDoNotAllowPads(True); z.SetDoNotAllowFootprints(True); z.SetDoNotAllowZoneFills(True)
    z.SetZoneName("SuperMini antenna copper keepout")
    x0,x1=35.052-6.35,35.052+6.35; y0,y1=15.24+6.35,15.24+11.26
    poly=z.Outline(); poly.NewOutline()
    for x,y in [(x0,y0),(x1,y0),(x1,y1),(x0,y1)]: poly.Append(vec(x,y))
    board.Add(z)

def add_ground_zone(board):
    z=pcbnew.ZONE(board); z.SetLayer(pcbnew.B_Cu); z.SetNet(netdict["GND"]); z.SetZoneName("GND plane")
    z.SetLocalClearance(mm(0.25)); z.SetThermalReliefGap(mm(0.25))
    z.SetThermalReliefSpokeWidth(mm(0.30)); z.SetMinThickness(mm(0.25)); z.SetAssignedPriority(0)
    poly=z.Outline(); poly.NewOutline()
    for x,y in [(0.75,0.75),(W_MM-0.75,0.75),(W_MM-0.75,H_MM-0.75),(0.75,H_MM-0.75)]: poly.Append(vec(x,y))
    board.Add(z)

def pad_key(pad):
    return (pad.GetParentFootprint().GetReferenceAsString(), str(pad.GetNumber()))

def build_obstacles(board):
    w=round(W_MM/STEP_MM)+1; h=round(H_MM/STEP_MM)+1
    track_blocked=[{},{}]; pad_items=[]
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            p=pad.GetPosition(); x=round(p.x/SCALE/STEP_MM); y=round(p.y/SCALE/STEP_MM)
            size=pad.GetSize()
            radius=math.ceil((max(size.x,size.y)/SCALE/2+CLEARANCE_MM+TRACE_MM/2)/STEP_MM)
            pad_items.append((x,y,radius,pad.GetNetname() or "",pad_key(pad)))
    nets=set(p[3] for p in pad_items if p[3])
    pad_blocked_by_net={}
    for route_net in nets:
        blocked=set()
        for px,py,rad,pnet,_ in pad_items:
            if pnet==route_net: continue
            for dx in range(-rad,rad+1):
                for dy in range(-rad,rad+1):
                    if dx*dx+dy*dy<=rad*rad: blocked.add((px+dx,py+dy))
        pad_blocked_by_net[route_net]=blocked
    holes=[(round(x/STEP_MM),round(y/STEP_MM),math.ceil(2.25/STEP_MM)) for x,y in [(5.08,5.08),(W_MM-5.08,5.08),(5.08,H_MM-5.08),(W_MM-5.08,H_MM-5.08)]]
    antenna=(round((35.052-6.35)/STEP_MM),round((35.052+6.35)/STEP_MM),round((15.24+6.35)/STEP_MM),round((15.24+11.26)/STEP_MM))
    return w,h,track_blocked,pad_items,holes,antenna,pad_blocked_by_net

def build_route_path(start,goal,net,w,h,track_blocked,holes,antenna,pad_blocked):
    def inside(x,y): return 1<=x<w-1 and 1<=y<h-1
    def is_blocked(x,y,layer):
        if not inside(x,y): return True
        if antenna[0]<=x<=antenna[1] and antenna[2]<=y<=antenna[3]: return True
        if (x,y) in pad_blocked: return True
        if any(other!=net for other in track_blocked[layer].get((x,y),())): return True
        for hx,hy,hr in holes:
            if (x-hx)*(x-hx)+(y-hy)*(y-hy)<=hr*hr: return True
        return False
    sx,sy=start; gx,gy=goal; frontier=[]; dist={}; prev={}
    for st in [(sx,sy,0),(sx,sy,1)]: dist[st]=0; heapq.heappush(frontier,(0,st))
    dirs=[(-1,0,10),(1,0,10),(0,-1,10),(0,1,10),(-1,-1,14),(1,-1,14),(-1,1,14),(1,1,14)]
    def heur(x,y):
        dx=abs(x-gx); dy=abs(y-gy); return 14*min(dx,dy)+10*(max(dx,dy)-min(dx,dy))
    found=None
    while frontier:
        _,cur=heapq.heappop(frontier); x,y,layer=cur
        if (x,y)==(gx,gy): found=cur; break
        base=dist[cur]
        for dx,dy,cost in dirs:
            nx,ny=x+dx,y+dy
            if is_blocked(nx,ny,layer): continue
            if dx and dy and (is_blocked(x+dx,y,layer) or is_blocked(x,y+dy,layer)): continue
            nxt=(nx,ny,layer); nd=base+cost
            if nd<dist.get(nxt,10**12):
                dist[nxt]=nd; prev[nxt]=cur; heapq.heappush(frontier,(nd+heur(nx,ny),nxt))
        nl=1-layer
        if not is_blocked(x,y,nl):
            nxt=(x,y,nl); nd=base+40
            if nd<dist.get(nxt,10**12):
                dist[nxt]=nd; prev[nxt]=cur; heapq.heappush(frontier,(nd+heur(x,y),nxt))
    if found is None: return None
    path=[found]
    while path[-1] in prev: path.append(prev[path[-1]])
    path.reverse(); return path

def add_tracks(board,net,path,track_blocked):
    netinfo=netdict[net]
    i=0
    while i<len(path)-1:
        x0,y0,l0=path[i]; x1,y1,l1=path[i+1]
        if l0!=l1:
            via=pcbnew.PCB_VIA(board); via.SetPosition(vec(x0*STEP_MM,y0*STEP_MM))
            via.SetWidth(mm(VIA_MM)); via.SetDrill(mm(DRILL_MM)); via.SetLayerPair(LAYER_F,LAYER_B)
            via.SetNet(netinfo); board.Add(via)
            for layer in (0,1):
                for dx in range(-3,4):
                    for dy in range(-3,4):
                        if dx*dx+dy*dy<=9:
                            cell=(x0+dx,y0+dy); track_blocked[layer].setdefault(cell,set()).add(net)
            i+=1; continue
        dx=x1-x0; dy=y1-y0; j=i+1
        while j<len(path)-1:
            if path[j+1][2]!=l0: break
            ndx=path[j+1][0]-path[j][0]; ndy=path[j+1][1]-path[j][1]
            if (ndx,ndy)!=(dx,dy): break
            j+=1
        ex,ey,_=path[j]
        track=pcbnew.PCB_TRACK(board); track.SetStart(vec(x0*STEP_MM,y0*STEP_MM)); track.SetEnd(vec(ex*STEP_MM,ey*STEP_MM))
        track.SetWidth(mm(TRACE_MM)); track.SetLayer(LAYER_F if l0==0 else LAYER_B)
        track.SetNet(netinfo); board.Add(track)
        for k in range(i,j):
            ax,ay,_=path[k]; bx,by,_=path[k+1]
            count=max(abs(bx-ax),abs(by-ay),1)
            for t in range(count+1):
                px=round(ax+(bx-ax)*t/count); py=round(ay+(by-ay)*t/count)
                for ox in range(-3,4):
                    for oy in range(-3,4):
                        if ox*ox+oy*oy<=9:
                            cell=(px+ox,py+oy); track_blocked[l0].setdefault(cell,set()).add(net)
        i=j

def mark_existing_tracks(board,track_blocked):
    for item in board.GetTracks():
        net=item.GetNetname()
        if not net: continue
        if item.GetClass()=="PCB_VIA":
            p=item.GetPosition(); x=round(p.x/SCALE/STEP_MM); y=round(p.y/SCALE/STEP_MM)
            for layer in (0,1):
                for dx in range(-3,4):
                    for dy in range(-3,4):
                        if dx*dx+dy*dy<=9:
                            track_blocked[layer].setdefault((x+dx,y+dy),set()).add(net)
            continue
        a=item.GetStart(); b=item.GetEnd()
        x0=round(a.x/SCALE/STEP_MM); y0=round(a.y/SCALE/STEP_MM)
        x1=round(b.x/SCALE/STEP_MM); y1=round(b.y/SCALE/STEP_MM)
        layer=0 if item.GetLayer()==LAYER_F else 1
        count=max(abs(x1-x0),abs(y1-y0),1)
        for t in range(count+1):
            x=round(x0+(x1-x0)*t/count); y=round(y0+(y1-y0)*t/count)
            for dx in range(-3,4):
                for dy in range(-3,4):
                    if dx*dx+dy*dy<=9:
                        track_blocked[layer].setdefault((x+dx,y+dy),set()).add(net)

def route_board(board):
    w,h,track_blocked,pad_items,holes,antenna,pad_blocked=build_obstacles(board)
    nets={}
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            net=pad.GetNetname()
            if net: nets.setdefault(net,[]).append(pad)
    routed=[]; failed=[]
    for net in sorted(nets):
        if net in ("GND","/GND"): continue
        pads=nets[net]
        if len(pads)<2: continue
        pads.sort(key=lambda p:(p.GetParentFootprint().GetReferenceAsString()!="U1",p.GetParentFootprint().GetReferenceAsString(),str(p.GetNumber())))
        base=pads[0]; bp=base.GetPosition(); start=(round(bp.x/SCALE/STEP_MM),round(bp.y/SCALE/STEP_MM))
        for target in pads[1:]:
            tp=target.GetPosition(); goal=(round(tp.x/SCALE/STEP_MM),round(tp.y/SCALE/STEP_MM))
            path=build_route_path(start,goal,net,w,h,track_blocked,holes,antenna,pad_blocked[net])
            if path is None: failed.append(f"{net}: {pad_key(base)} -> {pad_key(target)}"); continue
            add_tracks(board,net,path,track_blocked); routed.append((net,pad_key(base),pad_key(target),len(path)))
    return routed,failed

def main():
    global netdict, footprints, symbol_uuids
    sch_text=(ROOT/"QUARK_SuperMini_Carrier.kicad_sch").read_text(encoding="utf-8")
    symbol_uuids={}
    for match in re.finditer(r'\(symbol\s+\(lib_id\s+"[^"]+"\).*?\(uuid\s+"([^"]+)"\).*?\(property\s+"Reference"\s+"([^"]+)"',sch_text,re.S):
        symbol_uuids[match.group(2)]=match.group(1)
    make_socket_footprint()
    board=pcbnew.BOARD(); board.SetCopperLayerCount(2); board.SetFileName(str(BOARD_PATH))
    board.GetDesignSettings().SetBoardThickness(mm(1.6))
    netdict={}
    for name in ["GND","3V3","GPIO0","GPIO1","GPIO3","GPIO4","GPIO5","GPIO6","GPIO10","LED_B","LED_R"]:
        net=pcbnew.NETINFO_ITEM(board,"/"+name); board.Add(net); netdict[name]=net; netdict["/"+name]=net
    for name in [
        "unconnected-(J2-Pin_3-Pad3)", "unconnected-(U1-5V-Pad1)",
        "unconnected-(U1-GPIO2-Pad6)", "unconnected-(U1-GPIO7-Pad11)",
        "unconnected-(U1-GPIO8-Pad12)", "unconnected-(U1-GPIO9-Pad13)",
        "unconnected-(U1-GPIO20-Pad15)", "unconnected-(U1-GPIO21-Pad16)",
    ]:
        net=pcbnew.NETINFO_ITEM(board,name); board.Add(net); netdict[name]=net
    footprints={}
    add_local_fp(board,"U1","ESP32-C3 SuperMini / 2x8 socket","ESP32C3_SuperMini_Socket_2x8",35.052,15.24,
      {"1":"unconnected-(U1-5V-Pad1)","2":"GND","3":"3V3","4":"GPIO4","5":"GPIO3","6":"unconnected-(U1-GPIO2-Pad6)","7":"GPIO1","8":"GPIO0","9":"GPIO5","10":"GPIO6","11":"unconnected-(U1-GPIO7-Pad11)","12":"unconnected-(U1-GPIO8-Pad12)","13":"unconnected-(U1-GPIO9-Pad13)","14":"GPIO10","15":"unconnected-(U1-GPIO20-Pad15)","16":"unconnected-(U1-GPIO21-Pad16)"},(35.052,28.8))
    add_fp(board,"J1","OLED: GND / 3V3 / SCL / SDA","Connector_PinHeader_2.54mm","PinHeader_1x04_P2.54mm_Vertical",30.48,45.212,
      {"1":"GND","2":"3V3","3":"GPIO1","4":"GPIO0"},(30.48,42.418))
    add_fp(board,"J2","Mode: AI / COM / SYS(open)","Connector_PinHeader_2.54mm","PinHeader_1x03_P2.54mm_Vertical",10.16,37.084,
      {"1":"GND","2":"GPIO6","3":"unconnected-(J2-Pin_3-Pad3)"},(10.16,34.5))
    add_fp(board,"J3","Passive piezo: GPIO / GND","Connector_PinHeader_2.54mm","PinHeader_1x02_P2.54mm_Vertical",59.944,37.084,
      {"1":"GPIO5","2":"GND"},(59.944,34.5))
    add_fp(board,"R1","330R","Resistor_THT","R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal",12.7,27.94,
      {"1":"GPIO4","2":"LED_R"},(16.51,24.892))
    add_fp(board,"D1","Red 3mm","LED_THT","LED_D3.0mm",22.86,27.94,{"1":"LED_R","2":"GND"},(22.86,31.242))
    add_fp(board,"R2","330R","Resistor_THT","R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal",50.8,27.94,
      {"1":"GPIO10","2":"LED_B"},(54.61,24.892))
    add_fp(board,"D2","Blue 3mm","LED_THT","LED_D3.0mm",60.96,27.94,{"1":"LED_B","2":"GND"},(60.96,31.242))
    add_fp(board,"SW1","Page / refresh","Button_Switch_THT","SW_TH_Tactile_Omron_B3F-100x",35.052,35.052,
      {"1":"GPIO3","2":"GND"},(35.052,42.164))
    add_fp(board,"C1","10uF","Capacitor_THT","C_Disc_D5.0mm_W2.5mm_P5.00mm",20.32,42.164,
      {"1":"3V3","2":"GND"},(20.32,39.37))
    add_fp(board,"C2","100nF","Capacitor_THT","C_Disc_D5.0mm_W2.5mm_P5.00mm",50.8,42.164,
      {"1":"3V3","2":"GND"},(50.8,39.37))
    for ref,x,y in [("H1",5.08,5.08),("H2",65.024,5.08),("H3",5.08,50.292),("H4",65.024,50.292)]:
        add_fp(board,ref,"Mounting hole M3 NPTH","MountingHole","MountingHole_3.2mm_M3",x,y,None,(x,y-2.3))
    add_outline(board); add_keepout(board); add_ground_zone(board)
    add_text(board,"QUARK",35.052,2.286,2.0)
    add_text(board,"ESP32-C3 SUPERMINI CARRIER",35.052,31.75,1.15,layer=pcbnew.F_Fab)
    for idx,txt in enumerate(["GND","3V3","SCL","SDA"]): add_text(board,txt,26.67,45.212+2.54*idx,0.85,justify="right")
    for idx,txt in enumerate(["AI","COM","SYS OPEN"]): add_text(board,txt,7.62,37.084+2.54*idx,0.85,justify="right")
    for idx,txt in enumerate(["GPIO5","GND"]): add_text(board,txt,62.484,37.084+2.54*idx,0.85,justify="left")
    add_text(board,"ANTENNA KEEP-OUT",35.052,27.686,0.75,layer=pcbnew.F_Fab)
    routes,failed=route_board(board)
    # The OLED ground pad sits beyond an isolated edge of the pour; link it to the plane by an auto-routed branch.
    w,h,track_blocked,pad_items,holes,antenna,pad_blocked=build_obstacles(board)
    mark_existing_tracks(board,track_blocked)
    p1=next(p for p in footprints["J1"].Pads() if str(p.GetNumber())=="1")
    p2=next(p for p in footprints["C1"].Pads() if str(p.GetNumber())=="2")
    pa,pb=p1.GetPosition(),p2.GetPosition()
    start=(round(pa.x/SCALE/STEP_MM),round(pa.y/SCALE/STEP_MM))
    goal=(round(pb.x/SCALE/STEP_MM),round(pb.y/SCALE/STEP_MM))
    path=build_route_path(start,goal,"/GND",w,h,track_blocked,holes,antenna,pad_blocked["/GND"])
    if path is None: print("UNROUTED GND branch J1.1 -> C1.2")
    else: add_tracks(board,"/GND",path,track_blocked); print("Routed OLED ground to GND plane.")
    print(f"Routed signal connections: {len(routes)}; failures: {len(failed)}")
    for f in failed: print("UNROUTED",f)
    try:
        pcbnew.ZONE_FILLER(board).Fill(board.Zones()); print("Filled GND plane and antenna clearance.")
    except Exception as exc: print(f"Zone fill warning: {exc}")
    pcbnew.SaveBoard(str(BOARD_PATH),board); print(BOARD_PATH)

if __name__=="__main__": main()
