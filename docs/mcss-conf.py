DOXYFILE = "mcss-Doxyfile"
THEME_COLOR = "#cb4b16"
FAVICON = "enviroDIY_Favicon.png"
LINKS_NAVBAR1 = [
    (
        "About",
        "index",
        [
            ('<a href="page_hardware.html">Hardware</a>',),
            ('<a href="change_log.html">ChangeLog</a>',),
        ],
    ),
    ('<a href="classmodbus_master.html">The modbusMaster Class</a>', []),
    (
        "Examples",
        "page_the_examples",
        [
            ('<a href="example_read_write_register.html">Reading and Writing</a>',),
            ('<a href="example_scan_registers.html">Scanning Registers</a>',),
        ],
    ),
    (
        "Source Files",
        "files",
        [],
    ),
]
LINKS_NAVBAR2 = []
VERSION_LABELS = True
CLASS_INDEX_EXPAND_LEVELS = 2

STYLESHEETS = [
    "css/m-EnviroDIY+documentation.compiled.css",
]
EXTRA_FILES = ["gp-desktop-logo.png", "gp-mobile-logo.png", "gp-scrolling-logo.png"]
DESKTOP_LOGO = "gp-desktop-logo.png"
MOBILE_LOGO = "gp-mobile-logo.png"
SCROLLING_LOGO = "gp-scrolling-logo.png"
