import { createSignal } from "solid-js";
import "./app.css";
import cpumain from "branchy-cpu";

export default function App() {
  return (
    <main>
      <h1>Hello world!</h1>
      <button class="increment" onClick={() => cpumain()}>Run "hello" emulated (check console)</button>
    </main>
  );
}
