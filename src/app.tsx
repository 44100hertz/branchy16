import { createSignal } from "solid-js";
import "./app.css";
import cpu_mod from "branchy-cpu";
import Terminal from "./components/Terminal";
import Display from "./components/Display";

import { assemble } from "./assembler/assembler";
import hello_world_branching from "./assembler/programs/hello_world_branching";

const cpu = await cpu_mod();

export default function App() {
  assemble(hello_world_branching);
  const [consoleText, setConsoleText] = createSignal('');

  let test = () => { };

  function appendChar(_addr: number, value: number) {
    setConsoleText(consoleText() + String.fromCharCode(value));
  }

  const pokePtr = cpu.addFunction(appendChar, 'vii');
  cpu._set_poke_callback(0, pokePtr);

  let interval: any;
  test = () => {
    if (interval) clearInterval(interval);
    cpu._cpu_init();
    cpu._write_branching_hello();
    interval = setInterval(() => {
      const running = cpu._cpu_step();
      if (!running) clearInterval(interval);
    }, 1);
  };

  return (
    <main>
      <Terminal consoleText={consoleText} />
      <Display cpu={cpu} />
      <button class="increment" onClick={() => test()}>Emulate "hello world"</button>
    </main>
  );
}

