-- Resolve links for the combined manual. Keep links to documents outside the
-- manual usable in installed HTML/ODT instead of inventing local anchors.
local source_headers = {}
local global_headers = {}

local function basename(path)
    return path:gsub("\\", "/"):match("([^/]+)$")
end

local function repository_path(path)
    local parts = {}
    for part in ("doc/" .. path):gmatch("[^/]+") do
        if part == ".." then
            table.remove(parts)
        elseif part ~= "." then
            table.insert(parts, part)
        end
    end
    return table.concat(parts, "/")
end

function Pandoc(doc)
    doc:walk { Header = function(header)
        table.insert(global_headers, header.identifier)
    end }
    local index = 1
    for _, path in ipairs(PANDOC_STATE.input_files) do
        local file = assert(io.open(path, "r"))
        local source = pandoc.read(file:read("*a"), "gfm+footnotes")
        file:close()
        local anchors = {}
        source:walk { Header = function(header)
            anchors[header.identifier] = global_headers[index]
            if not anchors.first then anchors.first = global_headers[index] end
            index = index + 1
        end }
        source_headers[basename(path)] = anchors
    end
    return doc:walk { Link = function(el)
        if el.target:match("^[%a][%w+.-]*:") then return el end
        local path, suffix = el.target:match("^([^?#]+%.md)(.*)$")
        if not path then return el end
        local anchors = source_headers[basename(path)]
        if anchors and not path:match("^%.%./") then
            local fragment = suffix:match("^#(.+)$")
            local anchor = fragment and anchors[fragment] or anchors.first
            if anchor then el.target = "#" .. anchor end
        else
            el.target = "https://github.com/flamewing/asl-releases/blob/main/"
                .. repository_path(path) .. suffix
        end
        return el
    end }
end

if FORMAT:match 'pdf' or FORMAT:match 'latex' then
    function RawInline(raw)
        if raw.format == 'html' then
            if raw.text == '<br>' then
                return pandoc.RawInline('latex', '\\\\')
            elseif raw.text == '<sup>' then
                return pandoc.RawInline('latex', '$^{')
            elseif raw.text == '</sup>' then
                return pandoc.RawInline('latex', '}$')
            elseif raw.text == '<sub>' then
                return pandoc.RawInline('latex', '$_{')
            elseif raw.text == '</sub>' then
                return pandoc.RawInline('latex', '}$')
            end
        end
    end
end
