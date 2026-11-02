#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Source/model constraints supplement runtime tests; never access hardware."""
from pathlib import Path
import xml.etree.ElementTree as ET
import subprocess
import sys
import copy

APP = Path(__file__).resolve().parents[1]
REPO = APP.parents[1]
model = ET.parse(APP / 'model/diagnostic.qm').getroot()
original = ET.parse(REPO / 'examples/blinky-qm/model/blinky.qm').getroot()
# TASK-0026 intentionally extends the copied class, but preserves both heartbeat
# phase subtrees/actions/timers/diagram and initial 50% durations.
a = model.find(".//class[@name='BlinkyAO']")
b = original.find(".//class[@name='BlinkyAO']")
for phase in ('off', 'on'):
    path = f"statechart/state[@name='running']/state[@name='{phase}']"
    node = copy.deepcopy(a.find(path))
    transition = node.find("tran[@trig='TIMEOUT']")
    assert transition.get('target') == ('../../6' if phase == 'off' else '../../5')
    transition.set('target', '../../2' if phase == 'off' else '../../1')
    assert ET.tostring(node) == ET.tostring(b.find(path)), phase
assert a.find('statechart/initial').get('target') == '../1/5'
# QM requires transitions before child states and indexes both in paths. New
# handlers shift off/on to 5/6: explicitly check renumbering, not only XML text.
running = a.find("statechart/state[@name='running']")
items = [n for n in running if n.tag in ('tran', 'state', 'initial', 'choice')]
assert [n.get('trig') for n in items[:5]] == ['DUTY','RELAY_ON','FORCE_OFF','SESSION_LOST','RELAY_OFF']
assert [(n.tag,n.get('name')) for n in items[5:]] == [('state','off'),('state','on')]
constructor = a.find("operation[@name='BlinkyAO']/code").text
assert 'offTicks_(BSP::TICKS_PER_SECOND / 2U)' in constructor
assert 'onTicks_(BSP::TICKS_PER_SECOND / 2U)' in constructor
relay = model.find(".//class[@name='RelayTestHsm']/statechart/state")
assert [s.get('name') for s in relay.findall('state')] == ['idle', 'on']
assert relay.find("state[@name='idle']/tran[@trig='RELAY_ON']").get('target') == '../../3'
assert 'activateRelay' in relay.find("state[@name='on']/entry").text
assert 'activateRelay' not in relay.find("state[@name='idle']/entry").text
choice = relay.find("state[@name='on']/tran[@trig='RELAY_OFF']/choice")
assert choice.get('target') == '../../../2'
assert 'currentRelay_ ==' in choice.find('guard').text
assert 'RELAY_TIMEOUT' not in ET.tostring(model).decode()
assert 'TESTALL' not in ET.tostring(model).decode()
assert a.find("attribute[@name='relayTimeout_']") is None
assert relay.find("tran[@trig='SESSION_LOST']").get('target') == '../2'
assert relay.find("tran[@trig='FORCE_OFF']").get('target') == '../2'
diag = model.find(".//class[@name='DiagnosticAO']/statechart")
assert [s.get('name') for s in diag.findall('state')] == ['active']
assert [t.get('trig') for t in diag.findall('state/tran')] == ['SAMPLE', 'USB_SERVICE']
main = (APP / 'src/main.cpp').read_text()
steps = ['Carrier::initRelaysOff();', 'BSP::setFaultAction(', 'Carrier::initInputs();',
         'QP::QF::init();', 'BSP::init();', 'Blinky::start();', 'Diagnostic::start();']
positions = [main.index(step) for step in steps]
assert positions == sorted(positions), 'Startup ordering changed'
for p in (APP / 'src').glob('*'):
    text = p.read_text()
    assert 'hardware/' not in text and '#include "pico/' not in text, p
    assert 'gpio_' not in text, p
    if p.name != 'blinky.cpp' and p.name != 'blinky.hpp':
        assert 'requestDutyCycle(' not in text, p
assert 'command_parser' not in (APP / 'CMakeLists.txt').read_text()
assert '../blinky' not in (APP / 'CMakeLists.txt').read_text()
hooks = (REPO / 'bsp/common/qpcpp_hooks.cpp').read_text()
assert hooks.index('save_and_disable_interrupts()') < hooks.index('BSP::Detail::invokeFaultAction()') < hooks.index('for (;;)')
if len(sys.argv) > 1:
    carrier = REPO / 'bsp/carriers/pico_evb_rev_a'
    result = subprocess.run([sys.argv[1], '-std=c++17', '-fsyntax-only',
        '-DPICO_DEFAULT_LED_PIN=10', '-I' + str(APP / 'tests/stubs'),
        '-I' + str(carrier / 'include'), str(carrier / 'carrier.cpp')],
        capture_output=True, text=True, timeout=30)
    assert result.returncode != 0 and 'stock Pico 2 GP25 onboard LED' in result.stderr, result.stderr
print('Heartbeat preservation, generated relay structure, SDK/startup/fault contracts and unsafe LED override rejection passed.')
