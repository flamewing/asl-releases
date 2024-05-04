-- links-to-html.lua
function Link(el)
    el.target = string.gsub(el.target, "%.md", ".html")
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
