---------------------------------------------
-- Radare-Lua script to clean trash opcodes
--
-- Osu Tatakae! Sexy Pandas!
--
-- 2008            --pancake
---------------------------------------------


function opcleaner_configure()
  -- get sections
  print "Loading sections..."
  print ""
  r.cmd(".!rsc flag-sections ${FILE}");
  
  -- configure disassembler
  Radare.Config.set("asm.syntax", "intel")
  Radare.Config.set("asm.comments", "false")
  Radare.Config.set("asm.lines", "false")
  Radare.Config.set("asm.bytes", "false")
  Radare.Config.set("asm.offset", "false")
  Radare.Config.set("asm.flags", "false")
  Radare.Config.set("asm.split", "false")
  Radare.Config.set("asm.size", "true")
end

-- get opcode information
function opcleaner_update_opcode()
    opline = Radare.Print.dis(1, addr)
    opsize = tonumber(string.sub(opline,0,2))
    opcode = string.sub(opline, 3)
end

function opcleaner_range(from, to)
  addr = from
  while addr<to do
    opcleaner_update_opcode()

    if trace == 1 then
      print (string.format("0x%08x %s",addr+base_address, opcode))
    end

    -- PUSH + RET  =>  JMP
    if (string.match(opcode, "retn") and (string.match(old_opcode, "push"))) then
      jump = Radare.Print.hex(4, addr)
      Radare.Write.hex("e9"..jump.."90", addr)
      opcleaner_update_opcode()
      print (string.format("0x%08x: push+ret patched",addr))
    -- INC + DEC => NOP + NOP
    -- XXX: WARNING: does not detects inc eax dec edx !!!
    elseif
       (string.match(opcode, "inc") and (string.match(old_opcode, "dec"))) then
      jump = Radare.Print.hex(4, addr)
      Radare.Write.hex("90 90")
      opcleaner_update_opcode()
      print (string.format("0x%08x: inc+dec patched",addr))
    -- JZ + JNZ => 
    elseif
       ((string.match(opcode, "jz") and (string.match(old_opcode, "jnz")))
     or (string.match(opcode, "jnz") and (string.match(old_opcode, "jz")))) then
      jump = Radare.Print.hex(1, addr)
      Radare.Write.hex("eb", old_addr)
      Radare.Write.hex("90 90", addr)
      opcleaner_update_opcode()
      print (string.format("0x%08x: jz+jnz patched", addr))
    end
    old_opcode = opcode
    old_addr = addr;
    addr = addr + opsize
  end
end

-- analyze a section
function opcleaner_section(name)
  print("FROM: "..r.get("section_"..name))
  from = r.get("section_"..name)
  to   = r.get("section_"..name.."_end")
  old_opcode = ''

  print (string.format("Segment "..name.." at 0x%x",from))

  opcleaner_range(from, to)
end

------------------------------------------
-- MAIN ----------------------------------
------------------------------------------

print ""
print "OPCODE CLEANER FOR X86/RADARE"
print ""

trace = 1
base_address = 0x8048000

--Radare.debug("/bin/ls")
opcleaner_configure()
opcleaner_section("text")
--Radare.quit()

