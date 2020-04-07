function humanReadableTimeDiff(diff = 0) {
    return [
        ["day", Math.floor(Math.abs(diff) / 60 / 60 / 24)],
        ["hour", Math.floor(Math.abs(diff) / 60 / 60 % 24)],
        ["minute", Math.floor(Math.abs(diff) / 60 % 60)],
        ["second", Math.floor(Math.abs(diff) % 60)]
    ].filter(
        ([_, value]) => value > 0
    ).map(
        ([name, value]) => `${value} ${value == 1 ? name : name + "s"}`
    ).join(' ');
}

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

function timeDiffOfEvent(event)
{
    return parseInt(event["id"]) - Math.floor(Date.now() / 1000);
}

function createCountdown(event)
{
    let countdown = createTag("div", {"class": "countdown"});
    let diff = timeDiffOfEvent(event);

    if (diff >= 0) {
        countdown.innerHTML = `Starts in ${humanReadableTimeDiff(diff)}`;
    } else {
        countdown.innerHTML = `Ended ${humanReadableTimeDiff(diff)} ago`;
    }

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

function createEvent(event) {
    let eventTag = createTag("div", {
        "class": timeDiffOfEvent(event) < 0 ? "event past" : "event",
        "id": "_" + event["id"]
    });

    // TODO(#63): the frontend does not display a couple of past events like the legacy app
    // TODO(#64): the frontend does not display the current event with embeded twitch stream
    eventTag.appendChild(createTimestamp(event));
    eventTag.appendChild(createTitle(event));
    eventTag.appendChild(createCountdown(event));
    eventTag.appendChild(createChannel(event));
    // TODO(#65): markdown in the description is not renderered;
    eventTag.appendChild(createDescription(event));

    return eventTag;
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
