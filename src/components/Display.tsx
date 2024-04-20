import { createSignal, onMount } from "solid-js";
import * as cpu from "branchy-cpu";

const FPS = 60;
const MAX_FRAMESKIP = 10;

const SCREEN_WIDTH = 240;
const SCREEN_HEIGHT = 160;

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

const CYCLES_PER_HDRAW = 6;
const CYCLES_PER_HBLANK = 2;
const CYCLES_PER_HLINE = CYCLES_PER_HBLANK + CYCLES_PER_HDRAW;
const VBLANK_LINES = 40;

const ADDR_VBLANK_LOCK = 0xf100;
const ADDR_HBLANK_LOCK = 0xf102;
const ADDR_SCANLINE_COUNT = 0xf103;

const ADDR_BG_COLOR = 0x0f;

type Color = [number, number, number];

class Screen {
  ctx: CanvasRenderingContext2D;
  bgColor: Color;
  screenBuf: ImageData;
  ignore_poke: boolean = false;

  constructor(canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    this.bgColor = [0, 0, 0];
    this.screenBuf = new ImageData(
      new Uint8ClampedArray(SCREEN_HEIGHT * SCREEN_WIDTH * 4),
      SCREEN_WIDTH);

    // Set all alpha to 255
    for (let i = 0; i < this.screenBuf.data.length / 4; ++i) {
      this.screenBuf.data[i * 4 + 3] = 255;
    }

    cpu.setPokeHandler(1, this.handlePoke.bind(this));
  }

  writePixel(x: number, y: number, color: [number, number, number]) {
    const red_index = (y * SCREEN_WIDTH + x) * 4;
    for (let i = 0; i < 3; ++i) {
      this.screenBuf.data[red_index + i] = color[i];
    }
  }

  handlePoke(addr: number, value: number) {
    if (this.ignore_poke) return;
    if (addr === ADDR_BG_COLOR) {
      this.bgColor = Screen.wordToColor(value);
    }
  }

  runDisplayFrame(): boolean {
    for (let y = 0; y < SCREEN_HEIGHT; ++y) {
      cpu.ioStore(ADDR_SCANLINE_COUNT, y);
      // ignore pokes during scanline draw
      this.ignore_poke = true;
      // write to HBLANK lock 1 cycle early
      cpu.step(CYCLES_PER_HDRAW - 1);

      for (let x = 0; x < SCREEN_WIDTH; ++x) {
        this.writePixel(x, y, this.bgColor);
      }

      // write to HBLANK lock two cycles before
      cpu.ioStore(ADDR_HBLANK_LOCK, 0);
      cpu.step();
      // allow pokes during HBLANK
      this.ignore_poke = false;
      cpu.step(CYCLES_PER_HBLANK);
    }

    this.ctx.putImageData(this.screenBuf, 0, 0);

    // VBLANK
    this.ignore_poke = false;
    cpu.ioStore(ADDR_VBLANK_LOCK, 0);
    let running = cpu.step(VBLANK_LINES * CYCLES_PER_HLINE);
    return running;
  }

  static wordToColor(word: number): Color {
    return [
      (word >> 8 & 0xf) * 0x11,
      (word >> 4 & 0xf) * 0x11,
      (word >> 0 & 0xf) * 0x11,
    ];
  }
}

