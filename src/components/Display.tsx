import { createSignal } from "solid-js";
import * as cpu from "branchy-cpu";

const FPS = 60;
const MAX_FRAMESKIP = 10;

export default function Display(_props: {}) {
  const [frameCount, setFrameCount] = createSignal(0);
  const [stopped, setStopped] = createSignal(true);

  // Call runDisplayFrame at 60fps
  const frameLength = 1000 / FPS;
  let nextFrame: number;

  function animationFrame() {
    let running = true;

    let frames_ran = 0;
    let start_time = Date.now();

    while (start_time > nextFrame) {
      nextFrame += frameLength;
      if (frames_ran < MAX_FRAMESKIP) {
        running = runDisplayFrame();
        ++frames_ran;
      } else {
        nextFrame = Date.now();
        break;
      }
    }
    setFrameCount(frameCount() + frames_ran);

    if (running && !stopped()) {
      requestAnimationFrame(animationFrame);
    } else {
      setStopped(true);
    }
  }

  function runDisplay() {
    nextFrame = Date.now();
    cpu.loadDisplayBusyLoop();
    setStopped(true); // stop previous loop
    setFrameCount(0);
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
const VDRAW_LINES = 160;
const VBLANK_LINES = 40;
const ADDR_VBLANK_LOCK = 0xf100;
const ADDR_SCANLINE_COUNT = 0xf101;
const ADDR_BG_COLOR = 0xf10f;

function runDisplayFrame(): boolean {
  cpu.ioStore(ADDR_VBLANK_LOCK, 0);

  function handlePoke(addr: number, value: number) {
    // TODO: Handle I/O poke
    // The current busyloop just spams writes to ADDR_BG_COLOR
  }

  function ignorePoke(_addr: number, _value: number) { }

  for (let i = 0; i < VDRAW_LINES; ++i) {
    cpu.ioStore(ADDR_SCANLINE_COUNT, i);
    // ignore pokes during scanline draw
    cpu.setPokeHandler(1, ignorePoke);
    cpu.step(CYCLES_PER_HDRAW);


    // TODO: draw display line (in hardware this is would be parallel with the above cycles)


    // HBLANK
    // allow pokes during HBLANK
    cpu.setPokeHandler(1, handlePoke);
    cpu.step(CYCLES_PER_HBLANK);
  }
  // VBLANK
  cpu.setPokeHandler(1, handlePoke);
  let running = cpu.step(VBLANK_LINES * CYCLES_PER_HLINE);
  return running;
}

