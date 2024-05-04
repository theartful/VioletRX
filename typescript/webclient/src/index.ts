import { createPromiseClient } from "@connectrpc/connect";
import { createConnectTransport } from "@connectrpc/connect-web";
import { Rx } from "./gen/receiver_connect";
import init, { WebHandle } from "./gen/egui_client"
import wasmData from "./gen/egui_client_bg.wasm";

const transport = createConnectTransport({
    baseUrl: `http://${window.location.hostname}:50051`,
    useBinaryFormat: true,
});

const client = createPromiseClient(Rx, transport);

console.debug("Loading wasm…");
init(wasmData).then(on_wasm_loaded).catch(on_error);

async function on_wasm_loaded() {
    console.debug("Wasm loaded. Starting app…");

    let handle = new WebHandle();

    function check_for_panic() {
        if (handle.has_panicked()) {
            console.error("The egui app has crashed");

            // The demo app already logs the panic message and callstack, but you
            // can access them like this if you want to show them in the html:
            // console.error(`${handle.panic_message()}`);
            // console.error(`${handle.panic_callstack()}`);

            document.getElementById("the_canvas_id")!.remove();
            document.getElementById("center_text")!.innerHTML = `
            <p>
                The egui app has crashed.
            </p>
            <p style="font-size:10px" align="left">
                ${handle.panic_message()}
            </p>
            <p style="font-size:14px">
                See the console for details.
            </p>
            <p style="font-size:14px">
                Reload the page to try again.
            </p>`;
        } else {
            let delay_ms = 1000;
            setTimeout(check_for_panic, delay_ms);
        }
    }

    check_for_panic();

    handle.start("the_canvas_id").then(on_app_started).catch(on_error);

    await client.setInputDevice({ value: "file=/home/theartful/IQData/interesting,freq=100e6,rate=14.122e6,repeat=true,throttle=true" });
    await client.start({});
    let fft_data = await client.getFftData({});
    let data = fft_data.fftFrame!;
    let data_arr = new Float32Array(data.data);

    handle.new_fft_frame(data_arr, data.timestamp!.seconds, data.timestamp!.nanos, data.centerFreq, data.sampleRate);
}

function on_app_started(_handle: any) {
    // Call `handle.destroy()` to stop. Uncomment to quick result:
    // setTimeout(() => { handle.destroy(); handle.free()) }, 2000)

    console.debug("App started.");
    document.getElementById("center_text")!.innerHTML = '';
}

function on_error(error: any) {
    console.error("Failed to start: " + error);
    document.getElementById("the_canvas_id")!.remove();
    document.getElementById("center_text")!.innerHTML = `
        <p>
            An error occurred during loading:
        </p>
        <p style="font-family:Courier New">
            ${error}
        </p>
        <p style="font-size:14px">
            Make sure you use a modern browser with WebGL and WASM enabled.
        </p>`;
}
