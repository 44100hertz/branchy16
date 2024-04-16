import { type Accessor, Show } from "solid-js";
import { createSignal } from "solid-js";

export default function Terminal(props: { consoleText: Accessor<string> }) {
  const [expand, setExpand] = createSignal(false);
  let floatingPane: HTMLDivElement | undefined;

  const toggleExpand = () => {
    setExpand(!expand());
    if (expand() && floatingPane) {
      floatingPane.scroll(0, 99999);
    }
  }
  const shortLen = 60;

  const shortText = () =>
    ((' ').repeat(shortLen) + props.consoleText())
      .slice(-shortLen)
      .replaceAll('\n', '↪');

  return (
    <>
      <div class="console" onClick={toggleExpand}>{shortText()}</div>
      <Show when={expand()}>
        <div class="console floating" ref={floatingPane}>
          {props.consoleText()}
        </div>
      </Show>
    </>
  );
}
