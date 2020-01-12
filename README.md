# libKitsunemimiProjectNetwork

![Gitlab pipeline status](https://img.shields.io/gitlab/pipeline/tobiasanker/libKitsunemimiProjectNetwork?label=build%20and%20test&style=flat-square)
![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/tobiasanker/libKitsunemimiProjectNetwork?label=version&style=flat-square)
![GitHub](https://img.shields.io/github/license/tobiasanker/libKitsunemimiProjectNetwork?style=flat-square)
![C++Version](https://img.shields.io/badge/c%2B%2B-14-blue?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Linux--x64-lightgrey?style=flat-square)

## Description

WIP: this repo should contain all functionality, which is necessary in all my specific projects. Its not in a usable state at the moment.

For now it contains the beginning of a simple data transfer protocol created by myself. I need something like this for my projects, which consist of multiple programs, which should interact wiht each other. It partially contains and as next should contain the following:

- Session-handling (init, resume, end)
- Heartbeat-Message
- Error-Messages
- Data transfer messages (single part and multi part messages (with pause, abort and resume))
