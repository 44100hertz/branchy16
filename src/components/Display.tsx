import { createSignal, onMount } from "solid-js";
import * as cpu from "branchy-cpu";

const FPS = 60;
const MAX_FRAMESKIP = 10;

const SCREEN_WIDTH = 240;
const SCREEN_HEIGHT = 160;
const CYCLES_PER_HDRAW = 6;
const CYCLES_PER_HBLANK = 2;
const CYCLES_PER_HLINE = CYCLES_PER_HBLANK + CYCLES_PER_HDRAW;
const VBLANK_LINES = 40;

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
      <button onClick={runDisplay}>Run Display</button>
      <button onClick={stopDisplay}>Stop Display</button>
      <canvas ref={canvas} width={SCREEN_WIDTH} height={SCREEN_HEIGHT} />
      <div>Frame Count: {frameCount()}</div>
      <div>Stopped: {stopped() ? 'yes' : 'no'}</div>
    </>
  )
}

const ADDR_VBLANK_LOCK = 0xf100;
const ADDR_SCANLINE_COUNT = 0xf101;
const ADDR_BG_COLOR = 0x0f;

type Color = [number, number, number];

class Screen {
  ctx: CanvasRenderingContext2D;
  bgColor: Color;
  scanline: Uint8ClampedArray;

  constructor(canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    this.bgColor = [0, 0, 0];
    this.scanline = new Uint8ClampedArray(SCREEN_WIDTH * 4);
  }

  writePixel(x: number, color: [number, number, number]) {
    const red_index = x * 4;
    this.scanline[red_index] = color[0];
    this.scanline[red_index + 1] = color[1];
    this.scanline[red_index + 2] = color[2];
    this.scanline[red_index + 3] = 255;
  }

  runDisplayFrame(): boolean {
    cpu.ioStore(ADDR_VBLANK_LOCK, 0);

    const handlePoke = (addr: number, value: number) => {
      if (addr === ADDR_BG_COLOR) {
        this.bgColor = Screen.wordToColor(value);
      }
    }

    const ignorePoke = (_addr: number, _value: number) => { }

    for (let y = 0; y < SCREEN_HEIGHT; ++y) {
      cpu.ioStore(ADDR_SCANLINE_COUNT, y);
      // ignore pokes during scanline draw
      cpu.setPokeHandler(1, ignorePoke);
      cpu.step(CYCLES_PER_HDRAW);

      for (let x = 0; x < SCREEN_WIDTH; ++x) {
        this.writePixel(x, this.bgColor);
      }

      const imgData = new ImageData(this.scanline, SCREEN_WIDTH);
      this.ctx.putImageData(imgData, 0, y);

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

  static wordToColor(word: number): Color {
    return [
      (word >> 8 & 0xf) << 4,
      (word >> 4 & 0xf) << 4,
      (word >> 0 & 0xf) << 4,
    ];
  }
}

