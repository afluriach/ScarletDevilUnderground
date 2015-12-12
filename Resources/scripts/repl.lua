repl = {}
repl._history = {}

function repl.last()
    if #repl.history == 0 then
        print("No history")
    else
        print("Statement " .. #repl._history .. repl._history[#repl._history])
    end
end

function repl.history(idx)
    if not idx then
        for i,cmd in ipairs(repl._history) do
            print("Statement " .. i .. cmd)
        end
    elseif idx >= 1 and idx <= #repl._history then
        print(idx, repl._history[idx])
    else
        print(idx .. ": no history") 
    end
end

function repl.set_target(target)
    repl._target = target
end

function repl.start()
    local buf = ""
    while true do
        local line = io.read('l')
        
        if line == "--cancel" then
            buf = ""
        elseif line == "--exit" then
            break
        else
            buf = buf .. '\n' .. line            
            result = load(buf)
            
            if result then
                --A complete statement has been entered
                if repl._target then
                    --Do not run it in this context.
                    dostring_in_inst(buf, repl._target)
                else
                    print(result())
                end
                table.insert(repl._history, buf)
                print("Statement " .. #repl._history)
                buf = ""
            end
        end
    end
end

repl.open = repl.start
open_repl = repl.start