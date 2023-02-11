<script lang="ts">
    import { onMount } from "svelte";
    export let motor: "L" | "R";

    export let ws: WebSocket;

    function slope(x: number) {
        return Math.min(Math.max(x, 0), 1);
    }

    function doubleSlope(x: number) {
        const width = 0.4;
        const gap = 0.05;
        return slope((x - gap) / width) - 1 + slope((x - gap - 0.5) / width);
    }

    function motorMapping(x: number) {
        return Math.round(255 * doubleSlope(x));
    }

    let control: HTMLElement;

    function handleTouch(e: TouchEvent) {
        e.preventDefault();
        if (e.targetTouches.length === 1) {
            controlAbsPos = e.targetTouches[0].clientY;
        }
    }

    function preventBehavior(e: Event) {
        e.preventDefault();
    }

    let controlAbsPos = 0;
    let controlPos = 0;
    $: if (control)
        controlPos = motorMapping(1 - controlAbsPos / control.clientHeight);

    onMount(() => {
        controlAbsPos = control.clientHeight / 2;
        setInterval(() => {
            // fetch(`/set?motor=${motor}&pwm=${controlPos}`);
            if (ws) ws.send(new Uint8Array([
                (controlPos < 0 ? 2 : 0) + (motor == "R" ? 1 : 0),
                Math.abs(controlPos)
            ]))
        }, 100);
    });
</script>

<div
    class="w-[50vw] h-full border-[1px] border-red-500 relative"
    bind:this={control}
    on:touchstart={handleTouch}
    on:touchmove={handleTouch}
    on:touchend={preventBehavior}
    on:touchcancel={preventBehavior}

    >
    <div class="text-center m-10">
        {controlPos}
    </div>
    <div
        class="w-full h-[1px] bg-blue-500 absolute"
        style="top: {controlAbsPos}px;"
    />

    <div class="w-full h-[1px] bg-red-500 absolute top-[5%]" />
    <div class="w-full h-[1px] bg-red-500 absolute top-[45%]" />
    <div class="w-full h-[1px] bg-red-500 absolute top-[55%]" />
    <div class="w-full h-[1px] bg-red-500 absolute top-[95%]" />
</div>
