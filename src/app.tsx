import { createSignal } from "solid-js";
import "./app.css";
import cpu_mod from "branchy-cpu";

export default function App() {
  const [term, setTerm] = createSignal('');
  let test = () => {};

  const appendChar = (c) => {
    setTerm(term() + String.fromCharCode(c));
    return c;
  };

  cpu_mod().then((cpu) => {
    const appendCharPtr = cpu.addFunction(appendChar, 'ii');
    cpu._override_putchar(appendCharPtr);
    test = () => {
        cpu._cpu_init();
        cpu._write_branching_hello();
        for (let i = 0; i < 100; ++i) {
            cpu._cpu_step();
        }
    };
  });

  return (
    <main>
      <h1>{term()}</h1>
      <button class="increment" onClick={() => test()}>Emulate "hello world"</button>
    </main>
  );
}
