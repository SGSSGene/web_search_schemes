let generator_listtag        = document.getElementById("generator_list");
let generator_min_errorstag  = document.getElementById("generator_min_errors");
let generator_max_errorstag  = document.getElementById("generator_max_errors");
let generator_errortag       = document.getElementById("generator_error");
let schemeinputtag           = document.getElementById("schemeinput");
let errorfieldtag            = document.getElementById("errorfield");
let canvastag                = document.getElementById("canvas");
let vis_partstag             = document.getElementById("vis_parts");
let vis_alphabettag          = document.getElementById("vis_alphabet");
let vis_validtag             = document.getElementById("vis_valid");
let vis_completetag          = document.getElementById("vis_complete");
let vis_nonredundanttag      = document.getElementById("vis_nonredundant");
let vis_nodecounttag         = document.getElementById("vis_nodecount");
let vis_weightednodecounttag = document.getElementById("vis_weightednodecount");


let search_scheme_reload = () => {
    let input = schemeinput.value + "\n";
    try {
        let parts = Number(vis_partstag.value);
        let alphabet = Number(vis_alphabettag.value);

        let data = Module.convertSearchSchemeToSvg(input, parts, alphabet);
        let isValid        = Boolean(Module.isSearchSchemeValid(input));
        let isComplete     = Boolean(Module.isSearchSchemeComplete(input));
        let isNonRedundant = Boolean(Module.isSearchSchemeNonRedundant(input));
        vis_validtag.value        = isValid;
        vis_completetag.value     = isComplete;
        vis_nonredundanttag.value = isNonRedundant;
        vis_validtag.classList.remove("failed");
        vis_completetag.classList.remove("failed");
        vis_nonredundanttag.classList.remove("failed");

        if (!isValid) vis_validtag.classList.add("failed");
        if (!isComplete) vis_completetag.classList.add("failed");
        if (!isNonRedundant) vis_nonredundanttag.classList.add("failed");

        let nodeCount = Module.nodeCount(input, parts, alphabet);
        let weightedNodeCount = Module.weightedNodeCount(input, parts, alphabet);
        vis_nodecounttag.value = nodeCount;
        vis_weightednodecounttag.value = weightedNodeCount;

        console.log(data.length);
        console.log(data);
        canvastag.innerHTML = data;
        errorfieldtag.innerHTML = "";
    } catch(err) {
        errorfieldtag.innerHTML = escapeHtml(err.message);
        console.log("error: " + err);
    }
}
let regenerate = () => {
    if (generator_min_errorstag.value > generator_max_errorstag.value) {
        generator_errortag.innerHTML = "error: min error can not be larger then max error";
        return;
    }
    if (generator_min_errorstag.value < 0) {
        generator_errortag.innerHTML = "error: min error can not be negative";
        return;
    }
    generator_errortag.innerHTML = "";
    let gen = generator_listtag.value;
    let minK = generator_min_errorstag.value;
    let maxK = generator_max_errorstag.value;
    console.log(`generator ${gen} for ${minK}-${maxK} errors`);
    schemeinputtag.value = Module.generateSearchScheme(gen, Number(minK), Number(maxK));
    search_scheme_reload();
}

generator_listtag.oninput       = regenerate;
generator_min_errorstag.oninput = regenerate;
generator_max_errorstag.oninput = regenerate;

vis_partstag.oninput = search_scheme_reload;
vis_alphabettag.oninput = search_scheme_reload;

function init() {
    let dialogtag = document.getElementById("dialog_loading");
    dialogtag.style.visibility = "hidden";

    let genList = Module.generatorList();
    for (let i = 0; i < genList.size(); ++i) {
        let name = genList.get(i);
        generator_listtag.innerHTML += `<option value="${name}">${name}</option>\n`;
    }
    regenerate();
}
function escapeHtml(unsafe) {
  return unsafe
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}

schemeinputtag.oninput = () => {
    console.log("hitting some key");
    search_scheme_reload();
}

window.onload = () => {
    checkAvailable = () => {
        if (typeof Module != "undefined") {
            if (typeof Module.generatorList != "undefined") {
                return true;
            }
        }
        return false;
    }
    let intervalId = null;
    intervalId = setInterval(() => {
        if (checkAvailable()) {
            clearInterval(intervalId);
            init();
        }
    }, 200);
}
