#
Import("env lib suffix")

headers = Split("crc32.h deflate.h inffast.h inffixed.h inflate.h inftrees.h trees.h zutil.h")

sources = Split("crc32.c deflate.c infback.c inflate.c inftrees.c trees.c uncompr.c zutil.c adler32.c compress.c")

p = str( Platform() )

e = env.Copy()

if p == "win32a":
  sources += Split("gvmat32c.c gvmat32.asm inffas32.asm")
  e.Append( CPPDEFINES = Split("NOOLDPENTIUMCODE NO_GZCOMPRESS NO_GZIP ASMV") )
  e.Append( ASFLAGS = Split("/DNOOLDPENTIUMCODE /DNO_GUNZIP") )

else:
  sources += ["inffast.c"]

e.StaticLibrary(lib + "/zlib" + suffix, ["zlib/" + x for x in sources])
                       
