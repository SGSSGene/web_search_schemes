let generator_listtag        = document.getElementById("generator_list");
let generator_min_errorstag  = document.getElementById("generator_min_errors");
let generator_max_errorstag  = document.getElementById("generator_max_errors");
let generator_errortag       = document.getElementById("generator_error");
let schemeinputtag           = document.getElementById("schemeinput");
let errorfieldtag            = document.getElementById("errorfield");
let canvastag                = document.getElementById("canvas");
let vis_partstag             = document.getElementById("vis_parts");
let vis_alphabettag          = document.getElementById("vis_alphabet");
let vis_editdistancetag      = document.getElementById("vis_editdistance");
let vis_validtag             = document.getElementById("vis_valid");
let vis_completetag          = document.getElementById("vis_complete");
let vis_nonredundanttag      = document.getElementById("vis_nonredundant");
let vis_nodecounttag         = document.getElementById("vis_nodecount");
let vis_weightednodecounttag = document.getElementById("vis_weightednodecount");

let scheme_name = {
    name: "custom",
    min_errors: 0,
    max_errors: 0
};

let search_scheme_reload = () => {
    let input = schemeinput.value + "\n";
    try {
        let parts = Number(vis_partstag.value);
        let alphabet = Number(vis_alphabettag.value);
        let editdistance = vis_editdistancetag.checked;

        let dataList              = Module.convertSearchSchemeToSvgList(input, parts, alphabet, editdistance);
        let isValid               = Module.isSearchSchemeValid(input);
        let isComplete            = Module.isSearchSchemeComplete(input);
        let isNonRedundant        = Module.isSearchSchemeNonRedundant(input);
        vis_validtag.value        = isValid;
        vis_completetag.value     = isComplete;
        vis_nonredundanttag.value = isNonRedundant;
        vis_validtag.classList.remove("failed");
        vis_completetag.classList.remove("failed");
        vis_nonredundanttag.classList.remove("failed");

        if (!isValid) vis_validtag.classList.add("failed");
        if (!isComplete) vis_completetag.classList.add("failed");
        if (!isNonRedundant) vis_nonredundanttag.classList.add("failed");

        let nodeCount = Module.nodeCount(input, parts, alphabet, editdistance);
        let weightedNodeCount = Module.weightedNodeCount(input, parts, alphabet, editdistance);
        vis_nodecounttag.value = nodeCount;
        vis_weightednodecounttag.value = weightedNodeCount;

        canvastag.innerHTML = "";
        for (let i = 0; i < dataList.size(); ++i) {
            canvastag.innerHTML += dataList.get(i);
        }
        errorfieldtag.innerHTML = "";
        let clearHovers = () => {
            for (let e of document.querySelectorAll('[data-hovered="true"]')) {
                e.removeAttribute("data-hovered");
            }
        }

        let circles = document.querySelectorAll("circle");
        for (let c of circles) {
            c.onmouseleave = (e) => {
                clearHovers();
            }
            c.onmouseenter = (e) => {
                clearHovers();
                let nodeName = e.target.getAttribute("data-node-name");
                console.log("selecting: " + ".child-of-" + nodeName);
                e.target.setAttribute("data-hovered", "true");
                for (let child of document.querySelectorAll(".child-of-" + nodeName)) {
                    child.setAttribute("data-hovered", "true");
                }
            }
            c.onclick = (e) => {
                let nodeName = e.target.getAttribute("data-node-name");
                if (e.target.getAttribute("data-selected") == "true") {
                    e.target.removeAttribute("data-selected");
                    for (let child of document.querySelectorAll(".child-of-" + nodeName)) {
                        child.removeAttribute("data-selected");
                    }
                } else {
                    e.target.setAttribute("data-selected", "true");
                    for (let child of document.querySelectorAll(".child-of-" + nodeName)) {
                        child.setAttribute("data-selected", "true");
                    }
                }
            }
        }
    } catch(err) {
        errorfieldtag.innerHTML = escapeHtml(getExceptionMessage(err).toString());
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

    scheme_name = {
        name: gen,
        min_errors: minK,
        max_errors: maxK
    };

    //console.log(`generator ${gen} for ${minK}-${maxK} errors`);
    schemeinputtag.value = Module.generateSearchScheme(gen, Number(minK), Number(maxK));
    search_scheme_reload();
}

generator_listtag.oninput       = regenerate;
generator_min_errorstag.oninput = regenerate;
generator_max_errorstag.oninput = regenerate;

vis_partstag.oninput = search_scheme_reload;
vis_alphabettag.oninput = search_scheme_reload;
vis_editdistancetag.oninput = search_scheme_reload;

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
    scheme_name.name = "custom";

    search_scheme_reload();
}

let downloadtag = document.getElementById("download");
downloadtag.onclick = () => {
    console.log("hallo welt");
    let file1data = "hallo welt\n";
    let file2data = "foobar\n";
    let enc = new TextEncoder();
    let zip = new JSZip();

    let input = schemeinput.value + "\n";
    let parts = Number(vis_partstag.value);
    let alphabet = Number(vis_alphabettag.value);
    let editdistance = vis_editdistancetag.checked;

    let dataList = Module.convertSearchSchemeToSvgList(input, parts, alphabet, editdistance);

    let filename = `${scheme_name.name}_${scheme_name.min_errors}-${scheme_name.max_errors}`
    for (let i = 0; i < dataList.size(); ++i) {
        zip.file(`${filename}-${i}.svg`, enc.encode(dataList.get(i)));
    }

    zip.generateAsync({type : "blob"})
    .then((zipData)=> {
        const link = document.createElement('a');
        link.href = window.URL.createObjectURL(zipData);
        link.download = `${filename}.zip`
        link.click();
    });

/*    enc.encode(file1data);
    TextEncoder
    new JSZip();*/
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
