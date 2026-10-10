-- links-to-html.lua
function Link(el)
    el.target = string.gsub(el.target, ".+%.md#", "#")
    if string.find(el.target, ".+%.md") then
        el.target = '#' .. string.gsub(el.target, ".md", "")
    end
    return el
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
