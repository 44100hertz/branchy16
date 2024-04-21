import { createSignal, onMount } from "solid-js";
import * as cpu from "branchy-cpu";

const FPS = 60;
const MAX_FRAMESKIP = 10;

export default function Display(_props: {}) {
  const [frameCount, setFrameCount] = createSignal(0);
  const [stopped, setStopped] = createSignal(true);

  // Call runDisplayFrame at 60fps
  const frameLength = 1000 / FPS;
  let nextFrame: number;
  let canvas: HTMLCanvasElement | undefined = undefined;
  let screen: cpu.Screen;

  onMount(() => {
    screen = new cpu.Screen(canvas!);
  })

  function animationFrame() {
    let running = true;

    let frames_ran = 0;
    let start_time = Date.now();

    while (start_time > nextFrame) {
      nextFrame += frameLength;
      if (frames_ran < MAX_FRAMESKIP) {
        screen.runFrame();
        ++frames_ran;
      } else {
        nextFrame = Date.now();
        break;
      }
    }
    setFrameCount(frameCount() + frames_ran);

    if (running && !stopped()) {
      requestAnimationFrame(() => animationFrame());
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
      <canvas ref={canvas} width={cpu.SCREEN_WIDTH} height={cpu.SCREEN_HEIGHT} />
      <button onClick={runDisplay}>Run Display</button>
      <button onClick={stopDisplay}>Stop Display</button>
      <div>Frame Count: {frameCount()}</div>
      <div>Stopped: {stopped() ? 'yes' : 'no'}</div>
    </>
  )
}

