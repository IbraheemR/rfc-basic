<script lang="ts">
    import { onMount } from "svelte";
    import Control from "./Control.svelte";

    let ws: WebSocket;


    onMount(() => {
        ws = new WebSocket("ws://" + location.host + "/ws"); 
    })
</script>

<Control motor="L" {ws}/>
<Control motor="R" {ws}/>