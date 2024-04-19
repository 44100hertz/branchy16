import { createSignal } from "solid-js";
import "./app.css";
import * as cpu from "branchy-cpu";
import Terminal from "./components/Terminal";
import Display from "./components/Display";

import { assemble } from "./assembler/assembler";
import hello_world_branching from "./assembler/programs/hello_world_branching";

await cpu.wasmInit();

export default function App() {

  const [consoleText, setConsoleText] = createSignal('');

  let test = () => { };

  function appendChar(_addr: number, value: number) {
    setConsoleText(consoleText() + String.fromCharCode(value));
  }

  cpu.setPokeHandler(0, appendChar);

  let interval: any;
  test = () => {
    if (interval) clearInterval(interval);
    const binary = assemble(hello_world_branching);
    cpu.loadBinary(binary);

    interval = setInterval(() => {
      const running = cpu.step();
      if (!running) clearInterval(interval);
    }, 1);
  };

  return (
    <main>
      <Terminal consoleText={consoleText} />
      <Display />
      <button class="increment" onClick={() => test()}>Emulate "hello world"</button>
    </main>
  );
}

