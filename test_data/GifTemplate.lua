
templ{"GifHeader", function()
    char{"tag", 3}
    char{"version", 3}
end}

templ{"Gif", function()
    GifHeader{"header"}
end}

Gif{"gif"}
