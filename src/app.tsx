import "./app.css";
import * as cpu from "branchy-cpu";
import Terminal from "~/components/Terminal";
import Display from "~/components/Display";

import stress_test from "~/assembler/programs/stress_test";
import { assemble } from "~/assembler/assembler";

await cpu.wasmInit();

export default function App() {
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

  return (
    <main>
      <Terminal />
      <Display />
      <button onClick={runBenchmark}>Run Benchmark</button>
    </main>
  );
}

