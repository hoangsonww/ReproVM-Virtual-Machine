#!/usr/bin/env ruby
# bundle final artifacts with metadata

require 'json'
require 'time'

output = ARGV[0] || "package.tar.gz"
manifest = {
  generated_at: Time.now.utc.iso8601,
  git_commit: `git rev-parse --short HEAD`.strip,
  files: Dir.glob("*.{bin,txt,md,sha,tar.gz}")
}

File.write("package.meta.json", JSON.pretty_generate(manifest))

# Create tarball
system("tar", "-czf", output, "report.md", "metrics.txt", "package.meta.json")
puts "Packaged into #{output}"
