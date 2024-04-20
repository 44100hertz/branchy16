import { type Accessor, Show } from "solid-js";
import { createSignal } from "solid-js";
import * as cpu from "branchy-cpu";

import hello_world_branching from "~/assembler/programs/hello_world_branching";
import { assemble } from "~/assembler/assembler";

const ADDR_PUTCHAR = 0x00;

export default function Terminal(_props: {}) {
  // handle character input from CPU
  const [consoleText, setConsoleText] = createSignal('');
  function handlePoke(addr: number, value: number) {
    if (addr == ADDR_PUTCHAR) {
      setConsoleText(consoleText() + String.fromCharCode(value));
    }
  }
  cpu.setPokeHandler(0, handlePoke);

  // hello, world test program
  let interval: any;
  let testHello = () => {
    if (interval) clearInterval(interval);
    const binary = assemble(hello_world_branching);
    cpu.loadBinary(binary);

    interval = setInterval(() => {
      const running = cpu.step();
      if (!running) clearInterval(interval);
    }, 1);
  };

  // one-line terminal
  const shortLen = 60;
  const shortText = () =>
    ((' ').repeat(shortLen) + consoleText())
      .slice(-shortLen)
      .replaceAll('\n', '↪');


  // expanded terminal
  const [expand, setExpand] = createSignal(false);
  let floatingPane: HTMLDivElement | undefined;
  const toggleExpand = () => {
    setExpand(!expand());
    if (expand() && floatingPane) {
      floatingPane.scroll(0, 99999);
    }
  }

  return (
    <>
      <div class="console" onClick={toggleExpand}>{shortText()}</div>
      <Show when={expand()}>
        <div class="console floating" ref={floatingPane}>
          {consoleText()}
        </div>
      </Show>
      <button onClick={testHello}>Emulate "hello world"</button>
    </>
  );
}
