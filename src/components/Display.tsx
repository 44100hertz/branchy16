import { createSignal } from "solid-js";
import type { Cpu } from "branchy-cpu";

const FPS = 60;

export default function Display(props: { cpu: Cpu }) {
  const [frameCount, setFrameCount] = createSignal(0);
  const [stopped, setStopped] = createSignal(true);

  // Call runDisplayFrame at 60fps
  const frameLength = 1000 / 60;
  let nextFrame: number;

  function animationFrame() {
    let running = true;
    if (!nextFrame) nextFrame = Date.now();
    while (Date.now() > nextFrame) {
      nextFrame += frameLength;
      running = runDisplayFrame(props.cpu);
      setFrameCount(frameCount() + 1);
    }
    if (running && !stopped()) {
      requestAnimationFrame(animationFrame);
    } else {
      setStopped(true);
    }
  }

  function runDisplay() {
    nextFrame = Date.now();
    props.cpu._cpu_init();
    props.cpu._write_display_busyloop();
    setStopped(true); // stop previous loop
    requestAnimationFrame(() => {
      setStopped(false);
      animationFrame();
    });
  }

  function stopDisplay() {
    setStopped(true);
  }

  return (
    <>
      <button onClick={runDisplay}>Run Display</button>
      <button onClick={stopDisplay}>Stop Display</button>
      <div>TODO: Implement Display Here</div>
      <div>Frame Count: {frameCount()}</div>
      <div>Stopped: {stopped() ? 'yes' : 'no'}</div>
    </>
  )
}

const CYCLES_PER_HDRAW = 6;
const CYCLES_PER_HBLANK = 2;
const CYCLES_PER_HLINE = CYCLES_PER_HBLANK + CYCLES_PER_HDRAW;
const VDRAW_LINES = 240;
const VBLANK_LINES = 80;
const ADDR_VBLANK_LOCK = 0xf100;
const ADDR_SCANLINE_COUNT = 0xf100;
const ADDR_BG_COLOR = 0x00f;

function runDisplayFrame(cpu: Cpu): boolean {
  cpu._io_store(ADDR_VBLANK_LOCK, 0);

  function handlePoke(addr: number, value: number) {
    // TODO: Handle I/O poke
    // The current busyloop just spams writes to ADDR_BG_COLOR
  }

  function ignorePoke(_addr: number, _value: number) { }
  const handlePokePtr = cpu.addFunction(handlePoke, "vii");
  const ignorePokePtr = cpu.addFunction(ignorePoke, "vii");

  for (let i = 0; i < VDRAW_LINES; ++i) {
    cpu._io_store(ADDR_SCANLINE_COUNT, i);
    // ignore pokes during scanline draw
    cpu._set_poke_callback(1, ignorePokePtr);
    cpu._cpu_step_multiple(CYCLES_PER_HDRAW);


    // TODO: draw display line (in hardware this is would be parallel with the above cycles)


    // HBLANK
    // allow pokes during HBLANK
    cpu._set_poke_callback(1, handlePokePtr);
    cpu._cpu_step_multiple(CYCLES_PER_HBLANK);
  }
  // VBLANK
  cpu._set_poke_callback(1, handlePokePtr);
  let running = cpu._cpu_step_multiple(VBLANK_LINES * CYCLES_PER_HLINE);
  return running;
}

