templ{"NpkHeader", function()
    char{"tag", 4}
    dword{"blockLen"}
    dword{"dataOffset"}
end}

templ{"NpkFileEntry", function(dataOffset)
    dword{"blockLen"}
    dword{"entryFileOffset"}
    dword{"entryFileLength"}
    word{"entryNameLen"}
    char{"entryName", val{"entryNameLen"}}

--    local pos = bd:getPosition()
--    bd:setPosition(dataOffset + val{"entryFileOffset"})
--    
--    uchar{"data", val{"entryFileLength"}}
--    
--    bd:setPosition(pos)
end}

--function NpkEntry(dataOffset) end

templ{"NpkDirEntry", function(dataOffset)
    dword{"blockLen"}
    word{"entryNameLen"}
    char{"entryName", val{"entryNameLen"}}

    local subEntriesCount = 0
    while true do
        NpkEntry{"entry", {dataOffset}, open = true}
        if val{"entry.tag"} == "DNED" then
            break
        end
        subEntriesCount = subEntriesCount + 1
    end
end}

templ{"NpkEntry", function(dataOffset)
    char{"tag", 4}
    local tag = val{"tag"}
    if tag == "_RID" then -- DIR_
        NpkDirEntry{"dir", {dataOffset}, open = true}
    elseif tag == "ELIF" then -- FILE
        NpkFileEntry{"file", {dataOffset}, open = true}
    elseif tag == "DNED" then -- DEND
        dword{"marker"} -- 0
    end
end}

templ{"NpkDataHeader", function()
    char{"tag", 4} -- ATAD-DATA
    dword{"size"}
end}

templ{"Npk", function()
    NpkHeader{"npkHeader"}
    NpkEntry{"npkEntries", {val{"npkHeader.dataOffset"} + 8}, open = true}
    NpkDataHeader{"npkDataHeader"}
end}

Npk{"npk"}
