import { createSignal, onMount } from "solid-js";
import * as cpu from "branchy-cpu";
import Screen, { SCREEN_WIDTH, SCREEN_HEIGHT } from "~/components/Display/Screen";

const FPS = 60;
const MAX_FRAMESKIP = 10;

export default function Display(_props: {}) {
  const [frameCount, setFrameCount] = createSignal(0);
  const [stopped, setStopped] = createSignal(true);

  // Call runDisplayFrame at 60fps
  const frameLength = 1000 / FPS;
  let nextFrame: number;
  let canvas: HTMLCanvasElement | undefined = undefined;
  let screen: Screen | undefined = undefined;

  onMount(() => {
    screen = new Screen(canvas!);
  })

  function animationFrame(screen: Screen) {
    let running = true;

    let frames_ran = 0;
    let start_time = Date.now();

    while (start_time > nextFrame) {
      nextFrame += frameLength;
      if (frames_ran < MAX_FRAMESKIP) {
        running = screen.runDisplayFrame();
        ++frames_ran;
      } else {
        nextFrame = Date.now();
        break;
      }
    }
    setFrameCount(frameCount() + frames_ran);

    if (running && !stopped()) {
      requestAnimationFrame(() => animationFrame(screen));
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
      animationFrame(screen!);
    });
  }

  function stopDisplay() {
    setStopped(true);
  }

  return (
    <>
      <canvas ref={canvas} width={SCREEN_WIDTH} height={SCREEN_HEIGHT} />
      <button onClick={runDisplay}>Run Display</button>
      <button onClick={stopDisplay}>Stop Display</button>
      <div>Frame Count: {frameCount()}</div>
      <div>Stopped: {stopped() ? 'yes' : 'no'}</div>
    </>
  )
}

