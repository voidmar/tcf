project "tcf"
    location "."
    language "C++"
    kind "ConsoleApp"
    --entrypoint "mainCRTStartup"

    files
    {
        "**.cpp",
        "**.h",
    }

    links
    {
        "userenv"
    }
