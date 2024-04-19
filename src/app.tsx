import { createSignal } from "solid-js";
import "./app.css";
import * as cpu from "branchy-cpu";
import Terminal from "./components/Terminal";
import Display from "./components/Display";

import { assemble } from "./assembler/assembler";
import hello_world_branching from "./assembler/programs/hello_world_branching";
import stress_test from "./assembler/programs/stress_test";

await cpu.wasmInit();

export default function App() {

  const [consoleText, setConsoleText] = createSignal('');

  let test = () => { };

  function appendChar(_addr: number, value: number) {
    setConsoleText(consoleText() + String.fromCharCode(value));
  }

  function runBenchmark() {
    const cycles = 1_000_000;
    const desired_clock_speed = 96_000;

    cpu.loadBinary(assemble(stress_test));
    const start = Date.now();
    cpu.step(cycles);
    const run_time = Date.now() - start;
    const clock_speed = cycles / (run_time / 1000);

    console.log('runtime: ', run_time);
    console.log('desired clock speed: ', desired_clock_speed);
    console.log('stress test clock speed: ', clock_speed);
    console.log('speed: ', clock_speed / desired_clock_speed * 100, '%');
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
      <button onClick={runBenchmark}>Run Benchmark</button>
    </main>
  );
}

