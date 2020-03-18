[![Tsoding](https://img.shields.io/badge/twitch.tv-tsoding-purple?logo=twitch&style=for-the-badge)](https://www.twitch.tv/tsoding)

# Skedudle

Simple Event Schedule Web Application in C.

**WARNING! The application is in an active development state and is
not even alpha yet. Use it at your own risk. Nothing is documented,
anything can be changed at any moment or stop working at all.**

## Goals

This project has two goals:
1. Implement backend for https://github.com/tsoding/schedule
2. Implement enough Web related code to later extract it as an epic Web Framework in C

## Quick Start

```console
$ make
$ ./skedudle ./schedule.json 6969
$ <browser> http://localhost:6969
```

## Support

You can support my work via

- Twitch channel: https://www.twitch.tv/subs/tsoding
- Patreon: https://www.patreon.com/tsoding

## References

- https://github.com/stedolan/jq/blob/9b51a0852a0f91fbc987f5f2b302ff65e22f6399/src/jv_parse.c#L455
