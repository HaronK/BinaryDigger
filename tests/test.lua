
NpkHeader = templ{function()
    char{"tag", 4}
    dword{"blockLen"}
    dword{"dataOffset"}
end}

NpkFileEntry = templ(function(p)
    dword("blockLen")
    dword("entryFileOffset")
    dword("entryFileLength")
    word("entryNameLen")
    char("entryName", entryNameLen())

    pos = getPosition()
    setPosition(p[0] + entryFileOffset);
    uchar("data", entryFileLength)
    setPosition(pos);
end)

NpkEntry = function(p) end

NpkDirEntry = templ(function(p)
    dword{"blockLen"}
    word("entryNameLen")
    char("entryName", entryNameLen)

--    subEntriesCount = 0
    while true do
        NpkEntry("entry", {p, {open=true, read=function(data) return data.tag() end}})
        if entry.tag() == "DNED" then
            break
--        subEntriesCount++
    end
end)

NpkEntry = templ(function(p)
    char("tag", 4)
    if self.tag() == "_RID" then -- DIR_
        NpkDirEntry("dir", {p}) -- <open=true>;
    elseif self.tag() == "ELIF" then // FILE
        NpkFileEntry("file", {p}) -- <open=true>;
    elseif self.tag() == "DNED" then -- DEND
        dword("marker") -- 0
end)

NpkDataHeader = templ(function()
    char("tag", 4) -- ATAD-DATA
    dword("size")
end, read=function(t) return t.tag() end)

Npk = templ(function()
    NpkHeader("npkHeader")
    NpkEntry("npkEntries", {npkHeader.dataOffset + 8}) -- <open=true>;
    NpkDataHeader("npkDataHeader")
end)

register_templ(Npk)
