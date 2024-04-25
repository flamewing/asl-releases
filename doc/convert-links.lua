-- links-to-html.lua
function Link(el)
    el.target = string.gsub(el.target, ".+%.md#", "#")
    el.target = string.gsub(el.target, ".+%.md", "")
    return el
end

if FORMAT:match 'pdf' then
    function RawInline(raw)
        if raw.format == 'html' and raw.text == '<br>' then
            return pandoc.RawInline('pdf', '')
        end
    end
end

if FORMAT:match 'html' then
    function RawInline(raw)
        if raw.format == 'tex' and raw.text == '\\newline' then
            return pandoc.RawInline('html', '')
        end
    end
end
