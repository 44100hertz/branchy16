import * as cpu from "branchy-cpu";

export const SCREEN_WIDTH = 240;
export const SCREEN_HEIGHT = 160;

const CYCLES_PER_HDRAW = 6;
const CYCLES_PER_HBLANK = 2;
const CYCLES_PER_HLINE = CYCLES_PER_HBLANK + CYCLES_PER_HDRAW;
const VBLANK_LINES = 40;

const ADDR_VBLANK_LOCK = 0xf100;
const ADDR_HBLANK_LOCK = 0xf102;
const ADDR_SCANLINE_COUNT = 0xf103;

const ADDR_BG_COLOR = 0x0f;

type Color = [number, number, number];


export default class Screen {
  ctx: CanvasRenderingContext2D;
  bgColor: Color;
  bgBuffer: ImageBuffer;
  ignore_poke: boolean = false;

  constructor(canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext("2d")!;
    this.bgColor = [0, 0, 0];
    this.bgBuffer = new ImageBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, true);

    cpu.setPokeHandler(1, this.handlePoke.bind(this));
  }

  handlePoke(addr: number, value: number) {
    if (this.ignore_poke) return;
    if (addr === ADDR_BG_COLOR) {
      this.bgColor = ImageBuffer.wordToColor(value);
    }
  }

  runDisplayFrame(): boolean {
    for (let y = 0; y < SCREEN_HEIGHT; ++y) {
      cpu.ioStore(ADDR_SCANLINE_COUNT, y);
      // ignore pokes during scanline draw
      this.ignore_poke = true;
      // write to HBLANK lock 1 cycle early
      cpu.step(CYCLES_PER_HDRAW - 1);

      this.bgBuffer.writeLine(y, this.bgColor);

      // write to HBLANK lock two cycles before
      cpu.ioStore(ADDR_HBLANK_LOCK, 0);
      cpu.step();
      // allow pokes during HBLANK
      this.ignore_poke = false;
      cpu.step(CYCLES_PER_HBLANK);
    }

    this.ctx.putImageData(this.bgBuffer.buf, 0, 0);

    // VBLANK
    this.ignore_poke = false;
    cpu.ioStore(ADDR_VBLANK_LOCK, 0);
    let running = cpu.step(VBLANK_LINES * CYCLES_PER_HLINE);
    return running;
  }

}

class ImageBuffer {
  buf: ImageData;

  constructor(public width: number, public height: number, opaque: boolean) {
    const buf = new Uint8ClampedArray(width * height * 4);
    if (opaque) {
      for (let i = 0; i < buf.length / 4; ++i) {
        buf[i * 4 + 3] = 255;
      }
    }
    this.buf = new ImageData(buf, width);
  }

  static wordToColor(word: number): Color {
    const color = (word >> 8) / 255;
    const lum = (word & 0xff) / 255;
    const phases = [-0.3, -0.1, 0.5];
    const whitish = [1, .9, .8];
    const blackish = [.2, 0, .4];

    return Array(3).fill(0)
      .map((_: number, i: number) => Math.sin(6.28 * (color + phases[i])))
      .map((c: number, i: number) => Math.max(c, blackish[i]))
      .map((c: number, i: number) => (1 - lum) * c + lum * whitish[i])
      .map((c: number) => c * 255) as Color;
  }

  writeLine(y: number, color: Color) {
    for (let x = 0; x < SCREEN_WIDTH; ++x) {
      this.writePixel(x, y, color);
    }
  }

  writePixel(x: number, y: number, color: Color) {
    const red_index = (y * SCREEN_WIDTH + x) * 4;
    this.buf.data[red_index + 0] = color[0];
    this.buf.data[red_index + 1] = color[1];
    this.buf.data[red_index + 2] = color[2];
  }
}
