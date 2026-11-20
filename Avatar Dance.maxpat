{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 8,
      "minor": 6,
      "revision": 0,
      "architecture": "x64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      0,
      0,
      550,
      330
    ],
    "openinpresentation": 1,
    "boxes": [
      {
        "box": {
          "id": "view",
          "maxclass": "jsui",
          "patching_rect": [
            28,
            26,
            240,
            103
          ],
          "presentation": 1,
          "presentation_rect": [
            10,
            8,
            240,
            103
          ],
          "filename": "avatar-dance.js"
        }
      },
      {
        "box": {
          "id": "load",
          "maxclass": "message",
          "patching_rect": [
            28,
            136,
            99,
            22
          ],
          "presentation": 1,
          "presentation_rect": [
            10,
            118,
            99,
            22
          ],
          "text": "Choose folder\u2026"
        }
      },
      {
        "box": {
          "id": "builtin",
          "maxclass": "message",
          "patching_rect": [
            133,
            136,
            48,
            22
          ],
          "presentation": 1,
          "presentation_rect": [
            115,
            118,
            48,
            22
          ],
          "text": "Built-in"
        }
      },
      {
        "box": {
          "id": "steps",
          "maxclass": "number",
          "patching_rect": [
            188,
            136,
            39,
            22
          ],
          "presentation": 1,
          "presentation_rect": [
            170,
            118,
            39,
            22
          ],
          "minimum": 1,
          "maximum": 16
        }
      },
      {
        "box": {
          "id": "unit",
          "maxclass": "comment",
          "patching_rect": [
            229,
            136,
            47,
            22
          ],
          "presentation": 1,
          "presentation_rect": [
            211,
            118,
            47,
            22
          ],
          "text": "frm/beat"
        }
      },
      {
        "box": {
          "id": "status",
          "maxclass": "comment",
          "patching_rect": [
            28,
            161,
            244,
            22
          ],
          "presentation": 1,
          "presentation_rect": [
            10,
            143,
            244,
            22
          ],
          "text": "Built-in avatar"
        }
      },
      {
        "box": {
          "id": "dialog",
          "maxclass": "newobj",
          "patching_rect": [
            28,
            225,
            105,
            22
          ],
          "text": "opendialog fold"
        }
      },
      {
        "box": {
          "id": "path",
          "maxclass": "newobj",
          "patching_rect": [
            28,
            260,
            100,
            22
          ],
          "text": "prepend avatar"
        }
      },
      {
        "box": {
          "id": "reset",
          "maxclass": "message",
          "patching_rect": [
            145,
            225,
            49,
            22
          ],
          "text": "builtin"
        }
      },
      {
        "box": {
          "id": "speed",
          "maxclass": "newobj",
          "patching_rect": [
            215,
            225,
            100,
            22
          ],
          "text": "prepend steps"
        }
      },
      {
        "box": {
          "id": "ready",
          "maxclass": "newobj",
          "patching_rect": [
            335,
            25,
            95,
            22
          ],
          "text": "live.thisdevice"
        }
      },
      {
        "box": {
          "id": "start",
          "maxclass": "newobj",
          "patching_rect": [
            335,
            60,
            95,
            22
          ],
          "text": "t 1 b b"
        }
      },
      {
        "box": {
          "id": "init",
          "maxclass": "message",
          "patching_rect": [
            448,
            99,
            40,
            22
          ],
          "text": "init"
        }
      },
      {
        "box": {
          "id": "four",
          "maxclass": "message",
          "patching_rect": [
            396,
            99,
            29,
            22
          ],
          "text": "4"
        }
      },
      {
        "box": {
          "id": "clock",
          "maxclass": "newobj",
          "patching_rect": [
            335,
            136,
            80,
            22
          ],
          "text": "qmetro 33"
        }
      },
      {
        "box": {
          "id": "set",
          "maxclass": "newobj",
          "patching_rect": [
            28,
            193,
            92,
            22
          ],
          "text": "prepend set"
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "source": [
            "load",
            0
          ],
          "destination": [
            "dialog",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "dialog",
            0
          ],
          "destination": [
            "path",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "path",
            0
          ],
          "destination": [
            "view",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "builtin",
            0
          ],
          "destination": [
            "reset",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "reset",
            0
          ],
          "destination": [
            "view",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "steps",
            0
          ],
          "destination": [
            "speed",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "speed",
            0
          ],
          "destination": [
            "view",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "ready",
            0
          ],
          "destination": [
            "start",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "init",
            0
          ],
          "destination": [
            "view",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "four",
            0
          ],
          "destination": [
            "steps",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "clock",
            0
          ],
          "destination": [
            "view",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "view",
            0
          ],
          "destination": [
            "set",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "set",
            0
          ],
          "destination": [
            "status",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "start",
            0
          ],
          "destination": [
            "clock",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "start",
            1
          ],
          "destination": [
            "four",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "start",
            2
          ],
          "destination": [
            "init",
            0
          ]
        }
      }
    ]
  }
}
