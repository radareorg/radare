-- flag sections
r.cmd(".!rsc flag-sections $FILE")
r.cmd("eval file.baddr = 0x8048000")
r.cmd("seek section_text");
r.cmd("bsize section_text_end-section_text");
print("-------------");
print(r.cmd("pX"))
r.cmd("q")
