function createTag(name, attributes = {})
{
    let tag = document.createElement(name);
    for (let [key, value] of Object.entries(attributes)) {
        tag.setAttribute(key, value);
    }
    return tag;
}

function createTimestamp(json)
{
    let timestamp = createTag("div", {"class": "timestamp"});
    timestamp.innerHTML = `<a href="#_${json["id"]}">${json["id"]}</a>`;
    return timestamp;
}

function createTitle(json)
{
    let title = createTag("h1");
    title.innerHTML = `<a href="${json["url"]}">${json["title"]}</a>`;
    return title;
}

function createCountdown(json)
{
    let countdown = createTag("div", {"class": "countdown"});
    countdown.innerHTML = json["countdown"];
    return countdown;
}

function createChannel(json) {
    let channel = createTag("div", {"class": "channel"});
    channel.innerHTML = `<a href="${json["channel"]}">${json["channel"]}</a>`;
    return channel;
}

function createDescription(json) {
    let desc = createTag("div", {"class": "description markdown"});
    desc.innerHTML = json["description"];
    return desc;
}

function createEvent(json) {
    let event = createTag("div", {
        "class": "event",
        "id": "_" + json["id"]
    });

    // TODO(#63): the frontend does not display a couple of past events like the legacy app
    // TODO(#64): the frontend does not display the current event with embeded twitch stream
    event.appendChild(createTimestamp(json));
    event.appendChild(createTitle(json));
    event.appendChild(createCountdown(json));
    event.appendChild(createChannel(json));
    // TODO: markdown in the description is not renderered;
    event.appendChild(createDescription(json));

    return event;
}

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
