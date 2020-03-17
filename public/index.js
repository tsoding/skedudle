function createEntry(title, description) {
    let entry = document.createElement("div");
    entry.setAttribute("class", "entry");
    entry.innerHTML =
        `<span class="title">${title}:</span> `
        + `<span class="value">${description}</span>`;
    return entry;
}

function createEvent(json) {
    let event = document.createElement("div");
    event.setAttribute("class", "event");
    for (let key in json) {
        event.appendChild(createEntry(key, json[key]));
    }
    return event;
}

(() => {
    fetch("/next_stream")
        .then(res => res.json())
        .then(json => {
            document
                .querySelector("#app")
                .appendChild(createEvent(json));
        })
        .catch(err => {
            let app = document.querySelector("#app");
            app.innerText = "Bruh";
            console.error(err);
        });
})();
