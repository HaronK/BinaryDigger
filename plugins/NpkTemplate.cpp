/*
 * NdkTemplate.cpp
 *
 *  Created on: Jul 12, 2013
 *      Author: Oleg Khryptul
 */

#include <default_plugin.h>

TEMPL(NpkHeader)
    ARR(CHAR, tag, 4);
    VAR(DWORD, blockLen);
    VAR(DWORD, dataOffset);
TEMPL_END

TEMPL(NpkFileEntry, int dataOffset)
    VAR(DWORD, blockLen);
    VAR(DWORD, entryFileOffset);
    VAR(DWORD, entryFileLength);
    VAR(WORD,  entryNameLen);
    ARR(CHAR,  entryName, VAL(entryNameLen));

    bd_u64 pos = getPosition();
    setPosition(dataOffset + VAL(entryFileOffset));
    ARR(UCHAR, data, VAL(entryFileLength));
    setPosition(pos);
TEMPL_END

TEMPL_DECL(NpkEntry, int dataOffset)

TEMPL(NpkDirEntry, int dataOffset)
    VAR(DWORD, blockLen);
    VAR(WORD,  entryNameLen);
    ARR(CHAR,  entryName, VAL(entryNameLen));

    int subEntriesCount = 0;
    while (true)
    {
        VAR(NpkEntry, entry, dataOffset); // <open=true>;
        if (entry->getBlock("tag") == "DNED")
            break;
        subEntriesCount++;
    }
TEMPL_END

TEMPL_IMPL(NpkEntry, int dataOffset)
    ARR(CHAR, tag, 4);
    if (*tag == "_RID") // DIR_
    {
        VAR(NpkDirEntry, dir, dataOffset); // <open=true>;
    }
    else if (*tag == "ELIF") // FILE
    {
        VAR(NpkFileEntry, file, dataOffset); // <open=true>;
    }
    else if (*tag == "DNED") // DEND
    {
        VAR(DWORD, marker); // 0
    }
TEMPL_END

TEMPL(NpkDataHeader)
    ARR(CHAR, tag, 4); // ATAD-DATA
    VAR(DWORD, size);
TEMPL_END

TEMPL(Npk)
    VAR(NpkHeader, npkHeader);
    GET(npkHeader, DWORD, dataOffset);
    VAR(NpkEntry, npkEntries, dataOffset + 8); // <open=true>;
    VAR(NpkDataHeader, npkDataHeader);
TEMPL_END

PLUGIN(Npk)
    TEMPL_REGISTER(Npk);
PLUGIN_END

//TO_STRING(NpkFileEntry &data)
//{
//    return data.entryName;
//}

//string ReadNpkDirEntry(NpkDirEntry &data)
//{
//    return data.entryName;
//}

//string ReadNpkEntry(NpkEntry &data)
//{
//    string s, entryName;
//    if (data.tag == "_RID") // DIR_
//    {
//        entryName = data.dir.path + "/" + data.dir.entryName;
//    }
//    else if (data.tag == "ELIF") // FILE
//    {
//        entryName = data.file.path + "/" + data.file.entryName;
//    }
//    else if (data.tag == "DNED") // DEND
//    {
//       entryName = "<-";
//    }
//
//    SPrintf(s, "%c%c%c%c: %s", data.tag[3], data.tag[2], data.tag[1], data.tag[0], entryName);
//    return s;
//}

// Save unpacked structure

//local string INDENT = "    ";
//
//void ProcessEntry(NpkEntry &entry, string path, string indent);
//
//void ProcessFile(NpkFileEntry &file, string path, string indent)
//{
//    local string filePath = path + "/" + file.entryName;
//    local int fileId = FileNew(); //"", false
//    if (fileId != -1)
//    {
//        Printf("%s%s\n", indent, file.entryName);
//        FileSelect(fileId);
//        WriteBytes(file.data, 0, file.entryFileLength);
//        FileSave(filePath);
//        FileClose();
//    }
//    else
//        Warning("Cannot open file: %s", filePath);
//}
//
//void ProcessDir(NpkDirEntry &dir, string path, string indent)
//{
//    local string dirPath = path + "/" + dir.entryName;
//    MakeDir(dirPath);
//
//    Printf("%s%s[%d]:\n", indent, dir.entryName, dir.subEntriesCount);
//    local int i;
//    for (i = 0; i < dir.subEntriesCount; i++)
//        ProcessEntry(dir.entry[i], dirPath, indent + INDENT);
//}
//
//void ProcessEntry(NpkEntry &entry, string path, string indent)
//{
//    if (entry.tag == "_RID") // DIR_
//    {
//        ProcessDir(entry.dir, path, indent);
//    }
//    else if (entry.tag == "ELIF") // FILE
//    {
//        ProcessFile(entry.file, path, indent);
//    }
//}
//
//local string outDir = GetFileName() + ".dir";
//if (MessageBox(idYes|idNo, "Save structure",
//    "Save file structure in '%s' folder?", outDir) == idYes)
//{
//    MakeDir(outDir);
//
//    ProcessEntry(npkEntries, outDir, "");
//}
