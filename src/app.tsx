import { createSignal } from "solid-js";
import "./app.css";
import cpu_mod from "branchy-cpu";
import Terminal from "./Terminal";

export default function App() {
    const [consoleText, setConsoleText] = createSignal('');

    let test = () => { };

    function addChar(_addr: number, code: number) {
        setConsoleText(consoleText() + String.fromCharCode(code))
    }

    cpu_mod().then((cpu: any) => {
        const appendCharPtr = cpu.addFunction(addChar, 'vii');
        cpu._set_poke_callback(appendCharPtr);
        let interval: any;
        test = () => {
            if (interval) clearInterval(interval);
            cpu._cpu_init();
            cpu._write_branching_hello();
            interval = setInterval(() => {
                const running = cpu._cpu_step();
                if (running == 0) clearInterval(interval);
            }, 1);
        };
    });

    return (
        <main>
            <Terminal consoleText={consoleText} />
            <button class="increment" onClick={() => test()}>Emulate "hello world"</button>
        </main>
    );
}

