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

// TODO(#55): front page should look like https://tsoding.org/schedule/
(() => {
    let app = document.querySelector("#app");
    fetch("/api/period_streams")
        .then(res => res.json())
        .then(json => {
            for (let event in json) {
                app.appendChild(createEvent(json[event]));
            }
        })
        .catch(err => {
            app.innerText = "Bruh";
            console.error(err);
        });
})();
