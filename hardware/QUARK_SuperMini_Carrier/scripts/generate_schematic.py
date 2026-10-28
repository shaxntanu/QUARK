from pathlib import Path
from uuid import uuid4, uuid5, NAMESPACE_URL
import json

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "QUARK_SuperMini_Carrier.kicad_sch"
ROOT_UUID = str(uuid5(NAMESPACE_URL, 'QUARK_SuperMini_Carrier/root'))
FONT = '(effects (font (size 1.27 1.27)))'
def q(s): return json.dumps(str(s), ensure_ascii=False)
def write_clean(path, content):
    cleaned = "\n".join(line.rstrip() for line in content.splitlines()).rstrip()
    path.write_text(cleaned + "\n", encoding="utf-8")
def uid(): return str(uuid4())
def property_block(name, value, x, y, hide=False, justify="left"):
    hidden = " (hide yes)" if hide else ""
    justify_block = "" if justify == "center" else f" (justify {justify})"
    return f'(property {q(name)} {q(value)} (at {x:.4f} {y:.4f} 0) (effects (font (size 1.27 1.27)){justify_block}{hidden}))'

def pin(pin_type, number, name, x, y, angle=0, length=5.08):
    return f'''(pin {pin_type} line
      (at {x:.4f} {y:.4f} {angle})
      (length {length:.4f})
      (name {q(name)} (effects (font (size 1.0 1.0))))
      (number {q(number)} (effects (font (size 1.0 1.0))))
    )'''

def line(points, width=0.254):
    pts = " ".join(f"(xy {x:.4f} {y:.4f})" for x,y in points)
    return f'(polyline (pts {pts}) (stroke (width {width}) (type default)) (fill (type none)))'

def symbol_def(lib_name, unit_name, ref, value, footprint, pins, graphics, pin_offset=1.016, description=""):
    pin_text = "\n".join(pins)
    return f'''(symbol {q(lib_name)}
      (pin_names (offset {pin_offset}) (hide yes))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (duplicate_pin_numbers_are_jumpers no)
      {property_block("Reference", ref, 0, 5.08)}
      {property_block("Value", value, 0, 7.62)}
      {property_block("Footprint", footprint, 0, 10.16, True)}
      {property_block("Datasheet", "~", 0, 12.70, True)}
      {property_block("Description", description or value, 0, 0, True)}
      (symbol {q(unit_name)}
        {graphics}
        {pin_text}
      )
      (embedded_fonts no)
    )'''

# ESP32-C3 SuperMini module: USB-C toward pin-1 end; viewed from component side.
module_pins = []
left_names = ["5V", "GND", "3V3", "GPIO4", "GPIO3", "GPIO2", "GPIO1", "GPIO0"]
right_names = ["GPIO5", "GPIO6", "GPIO7", "GPIO8", "GPIO9", "GPIO10", "GPIO20", "GPIO21"]
left_types = ["power_in", "passive", "power_out"] + ["bidirectional"]*5
for i, (name, typ) in enumerate(zip(left_names, left_types), start=1):
    py = 8.89 - (i-1)*2.54
    module_pins.append(pin(typ, str(i), name, -17.78, py, 0))
for i, name in enumerate(right_names, start=9):
    py = 8.89 - (i-9)*2.54
    module_pins.append(pin("bidirectional", str(i), name, 17.78, py, 180))
module_graphics = '(rectangle (start -12.7 10.16) (end 12.7 -10.16) (stroke (width 0.254) (type default)) (fill (type background)))'
defs = [symbol_def("QUARK:ESP32C3_SuperMini", "ESP32C3_SuperMini_1_1", "U", "ESP32-C3 SuperMini", "QUARK:ESP32C3_SuperMini_Socket_2x8", module_pins, module_graphics, description="ESP32-C3 SuperMini 2x8 dev board, socket mounted")]
hole_def = symbol_def("QUARK:MountingHole", "MountingHole_1_1", "H", "Mounting hole M3 NPTH", "MountingHole:MountingHole_3.2mm_M3", [], '(circle (center 0 0) (radius 3.175) (stroke (width 0.254) (type default)) (fill (type none)))', description="Non-plated M3 mounting hole")
hole_def = hole_def.replace("(in_bom yes)", "(in_bom no)", 1)
defs.append(hole_def)

def two_pin_symbol(lib, unit, ref, value, footprint, p1, p2, graphic, description=""):
    pins = [pin(p1[0], p1[1], p1[2], -5.08, 0, 0), pin(p2[0], p2[1], p2[2], 5.08, 0, 180)]
    return symbol_def(lib, unit, ref, value, footprint, pins, graphic, description=description)

defs += [
    two_pin_symbol("QUARK:R", "R_1_1", "R", "R", "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal",
      ("passive","1","1"), ("passive","2","2"),
      line([(-2.54,0),(-1.778,1.016),(-0.762,-1.016),(0.254,1.016),(1.27,-1.016),(2.54,0)]), "Current-limiting resistor"),
    two_pin_symbol("QUARK:LED", "LED_1_1", "D", "LED", "LED_THT:LED_D3.0mm",
      ("passive","1","A"), ("passive","2","K"),
      '(rectangle (start -1.27 1.27) (end 1.27 -1.27) (stroke (width 0.254) (type default)) (fill (type none))) ' + line([(-0.6,-0.65),(0.65,0),( -0.6,0.65),(-0.6,-0.65)],0.20) + ' ' + line([(0.70,0.90),(1.35,1.55)],0.20) + ' ' + line([(0.28,1.32),(0.72,1.75)],0.20),
      "Through-hole LED"),
    two_pin_symbol("QUARK:C", "C_1_1", "C", "C", "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm",
      ("passive","1","1"), ("passive","2","2"),
      line([(-0.8,-1.52),(-0.8,1.52)],0.254) + ' ' + line([(0.8,-1.52),(0.8,1.52)],0.254),
      "Ceramic decoupling capacitor"),
    two_pin_symbol("QUARK:SW_Push", "SW_Push_1_1", "SW", "SW_Push", "Button_Switch_THT:SW_TH_Tactile_Omron_B3F-100x",
      ("passive","1","1"), ("passive","2","2"),
      '(circle (center -1.27 0) (radius 0.45) (stroke (width 0.20) (type default)) (fill (type none))) ' +
      '(circle (center 1.27 0) (radius 0.45) (stroke (width 0.20) (type default)) (fill (type none))) ' +
      line([(-0.8,0.2),(0.1,1.05),(0.8,1.05)],0.254),
      "Momentary normally-open push button")
]

def connector_def(count, name, footprint):
    # Pins are stacked top-to-bottom when viewed in the schematic.
    y_positions = {2:[1.27,-1.27],3:[2.54,0,-2.54],4:[3.81,1.27,-1.27,-3.81]}[count]
    pin_specs = []
    for idx, py in enumerate(y_positions, 1):
        pin_specs.append(pin("passive", str(idx), f"Pin_{idx}", -5.08, py, 0))
    half = max(1.27, (count-1)*1.27 + 1.27)
    graphics = f'(rectangle (start -1.27 {half:.4f}) (end 1.27 {-half:.4f}) (stroke (width 0.254) (type default)) (fill (type background)))'
    # small open rectangles make the connector unmistakable.
    graphics += " " + " ".join(f'(rectangle (start -1.27 {py+0.32:.4f}) (end 0 {py-0.32:.4f}) (stroke (width 0.1524) (type default)) (fill (type none)))' for py in y_positions)
    return symbol_def(f"QUARK:Conn_01x{count:02d}", f"Conn_01x{count:02d}_1_1", "J", name, footprint, pin_specs, graphics, description="External 2.54 mm pin header")

defs += [
    connector_def(4, "OLED: GND, 3V3, SCL, SDA", "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical"),
    connector_def(3, "Mode: AI, COM, SYS(open)", "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical"),
    connector_def(2, "Passive piezo: GPIO, GND", "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical"),
]


def placed(lib_id, ref, value, footprint, x, y, pin_numbers):
    sym_uuid = str(uuid5(NAMESPACE_URL, 'QUARK_SuperMini_Carrier/symbol/' + ref))
    if lib_id == "QUARK:ESP32C3_SuperMini":
        ref_y, value_y, field_justify = y-16.51, y-13.97, "center"
    elif lib_id.startswith("QUARK:Conn_"):
        ref_y, value_y, field_justify = y-10.16, y-7.62, "center"
    else:
        ref_y, value_y, field_justify = y-6.35, y-3.81, "center"
    props = [
        property_block("Reference", ref, x, ref_y, justify=field_justify),
        property_block("Value", value, x, value_y, justify=field_justify),
        property_block("Footprint", footprint, x, y, True),
        property_block("Datasheet", "~", x, y, True),
    ]
    pin_blocks = "\n".join(f'(pin {q(n)} (uuid {q(uid())}))' for n in pin_numbers)
    return f'''(symbol
      (lib_id {q(lib_id)})
      (at {x:.4f} {y:.4f} 0)
      (unit 1)
      (exclude_from_sim no)
      (in_bom {"no" if lib_id == "QUARK:MountingHole" else "yes"})
      (on_board yes)
      (dnp no)
      (fields_autoplaced yes)
      (uuid {q(sym_uuid)})
      {' '.join(props)}
      {pin_blocks}
      (instances (project "QUARK_SuperMini_Carrier"
        (path {q('/'+ROOT_UUID)} (reference {q(ref)}) (unit 1))
      ))
    )'''
def label(text, x, y, side="left"):
    justify = "left" if side == "left" else "right"
    return f'''(label {q(text)} (at {x:.4f} {y:.4f} 0)
      (effects (font (size 1.27 1.27)) (justify {justify} bottom))
      (uuid {q(uid())})
    )'''

def no_connect(x,y):
    return f'(no_connect (at {x:.4f} {y:.4f}) (uuid {q(uid())}))'

items=[]
# U1 module; pin coordinate transformation at symbol angle 0 is x+lib_x, y-lib_y.
ux,uy=100.33,104.14
items.append(placed("QUARK:ESP32C3_SuperMini","U1","ESP32-C3 SuperMini / 2x8 socket","QUARK:ESP32C3_SuperMini_Socket_2x8",ux,uy,[str(i) for i in range(1,17)]))
left_net = {2:"GND",3:"3V3",4:"GPIO4",5:"GPIO3",7:"GPIO1",8:"GPIO0"}
right_net = {9:"GPIO5",10:"GPIO6",14:"GPIO10"}
for i,name in enumerate(left_names, start=1):
    py=8.89-(i-1)*2.54; gx=ux-17.78; gy=uy-py
    if i in left_net: items.append(label(left_net[i],gx,gy,"right"))
    else: items.append(no_connect(gx,gy))
for i,name in enumerate(right_names, start=9):
    py=8.89-(i-9)*2.54; gx=ux+17.78; gy=uy-py
    if i in right_net: items.append(label(right_net[i],gx,gy,"left"))
    else: items.append(no_connect(gx,gy))

# Red and blue status LED branches.
parts = [
    ("QUARK:R","R1","330R","Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal",146.05,71.12,["1","2"]),
    ("QUARK:LED","D1","Red 3mm","LED_THT:LED_D3.0mm",167.64,71.12,["1","2"]),
    ("QUARK:R","R2","330R","Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal",146.05,91.44,["1","2"]),
    ("QUARK:LED","D2","Blue 3mm","LED_THT:LED_D3.0mm",167.64,91.44,["1","2"]),
    ("QUARK:SW_Push","SW1","Page / refresh","Button_Switch_THT:SW_TH_Tactile_Omron_B3F-100x",146.05,111.76,["1","2"]),
    ("QUARK:C","C1","10uF","Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm",146.05,132.08,["1","2"]),
    ("QUARK:C","C2","100nF","Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm",167.64,132.08,["1","2"]),
    ("QUARK:Conn_01x04","J1","OLED: GND / 3V3 / SCL / SDA","Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical",210.82,73.66,["1","2","3","4"]),
    ("QUARK:Conn_01x03","J2","Mode: AI / COM / SYS(open)","Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical",210.82,101.6,["1","2","3"]),
    ("QUARK:Conn_01x02","J3","Passive piezo: GPIO / GND","Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical",210.82,124.46,["1","2"]),
]
for p in parts:
    items.append(placed(*p))

for index, x in enumerate([52.07, 83.82, 115.57, 147.32], start=1):
    items.append(placed("QUARK:MountingHole", f"H{index}", "Mounting hole M3 NPTH", "MountingHole:MountingHole_3.2mm_M3", x, 177.8, []))

# Component terminal labels: left terminal at x-5.08, right at x+5.08.
def two_labels(x,y,left,right):
    return [label(left,x-5.08,y,"right"),label(right,x+5.08,y,"left")]
items += two_labels(146.05,71.12,"GPIO4","LED_R")
items += two_labels(167.64,71.12,"LED_R","GND")
items += two_labels(146.05,91.44,"GPIO10","LED_B")
items += two_labels(167.64,91.44,"LED_B","GND")
items += two_labels(146.05,111.76,"GPIO3","GND")
items += two_labels(146.05,132.08,"3V3","GND")
items += two_labels(167.64,132.08,"3V3","GND")
# Connector pin 1 is top; lib y is positive upward, converted to screen y by subtracting it.
for index, net in enumerate(["GND","3V3","GPIO1","GPIO0"]):
    lib_y=[3.81,1.27,-1.27,-3.81][index]
    items.append(label(net,210.82-5.08,73.66-lib_y,"right"))
for index, net in enumerate(["GND","GPIO6",None]):
    lib_y=[2.54,0,-2.54][index]
    if net: items.append(label(net,210.82-5.08,101.6-lib_y,"right"))
    else: items.append(no_connect(210.82-5.08,101.6-lib_y))
for index, net in enumerate(["GPIO5","GND"]):
    lib_y=[1.27,-1.27][index]
    items.append(label(net,210.82-5.08,124.46-lib_y,"right"))

# Notes make the active-low switch wiring and external OLED pin order explicit.
texts = [
    '(text "Firmware map: SDA=GPIO0, SCL=GPIO1, page=GPIO3, red=GPIO4, piezo=GPIO5, mode=GPIO6, blue=GPIO10." (at 40 139 0) (effects (font (size 1.27 1.27)) (justify left)))',
    '(text "Mode switch: AI shorts COM to GND; SYS leaves COM open for INPUT_PULLUP. OLED module is external." (at 40 143 0) (effects (font (size 1.27 1.27)) (justify left)))',
    '(text "Use a passive piezo disc on J3; do not connect a high-current magnetic buzzer directly to GPIO5." (at 40 147 0) (effects (font (size 1.27 1.27)) (justify left)))'
]
header = f'''(kicad_sch
 (version 20250114)
 (generator "eeschema")
 (generator_version "10.0")
 (uuid {q(ROOT_UUID)})
 (paper "A4")
 (title_block (title "QUARK ESP32-C3 SuperMini Carrier") (date "2026-10-28") (rev "A") (company "QUARK"))
 (lib_symbols
   {' '.join(defs)}
 )
 {' '.join(items)}
 {' '.join(texts)}
 (sheet_instances (path "/" (page "1")))
 (embedded_fonts no)
)
'''
lib_content = '(kicad_symbol_lib (version 20231120) (generator "kicad_symbol_editor") ' + " ".join(d.replace('(symbol "QUARK:', '(symbol "', 1) for d in defs) + ')\n'
write_clean(ROOT / "QUARK.kicad_sym", lib_content)
sym_table = '(sym_lib_table\n (lib (name "QUARK") (type "KiCad") (uri "' + chr(36) + '{KIPRJMOD}/QUARK.kicad_sym") (options "") (descr "QUARK carrier schematic symbols"))\n)\n'
write_clean(ROOT / "sym-lib-table", sym_table)
write_clean(OUT, header)
print(OUT)
print(f"schematic symbols: {len(parts)+1}; labels/no-connects: {sum(1 for x in items if x.startswith('(label')) + sum(1 for x in items if x.startswith('(no_connect'))}")
