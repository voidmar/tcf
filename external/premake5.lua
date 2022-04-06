
deps = os.matchdirs("*")
for i, dep in ipairs(deps) do
    if os.isfile(dep .. "/premake5.lua") then
        include(dep)
    end
end

deps = os.matchfiles("*.lua")
for i, dep in ipairs(deps) do
    if dep ~= "premake5.lua" then
        include(dep)
    end
end
