task :parser_tokens do
  tokens = ""
  
  File.open("include/menace/tokens.x", "r") do |f|
    ix = 1
    f.each_line do |l|
      if l =~ /T_(\w+),/
        tokens << "#{$1} = [\\#{sprintf("%03o", ix)}]\n"
        ix += 1
      end
    end
  end
  
  parser = File.read("src/parser.leg")
  
  state = :out
  File.open("src/parser.leg", "w") do |f|
    parser.each_line do |l|
      case state
      when :out
        f.write l
        if l =~ /# BEGIN-TOKENS/
          f.write tokens
          state = :in
        end
      when :in
        if l =~ /# END-TOKENS/
          f.write l
          state = :out
        end
      end
    end
  end
end