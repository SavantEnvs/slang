--max-hierarchy-depth=32 --error-limit 5 --compat vcs -T max --allow-use-before-declare
${HOME}/x.sv $(PWD)/y.sv -v lib.v --suppress-warnings "a b/*" --color-diagnostics
