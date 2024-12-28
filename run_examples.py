import subprocess
import os
import sys

srcdir = os.path.join(os.path.dirname(__file__), "examples")
builddir = os.path.join(sys.argv[1], "examples")

with open(os.path.join(sys.argv[1], "examples.dox"), "w", encoding="utf-8") as f:
    for name in sorted(list(os.listdir(srcdir))):
        if name.endswith(".cc"):
            print(name)
            stderr_lines = subprocess.run(
                [os.path.join(builddir, "y3c-example-" + name[:-3])],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                check=False,
            ).stdout.split("\n")
            f.write(
                "\n".join(
                    [
                        "/*!",
                        f"\\example {name}",
                        "> example output:",
                        "> ```",
                        *["> " + ln for ln in stderr_lines],
                        "> ```",
                        "*/",
                        "",
                    ]
                )
            )
