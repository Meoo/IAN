return {

  ian = {

    shell = {

      title = "IAN",
      ["title:dev"] = "IAN Dev",

      meta = {
        description = "Woof cap'n!",
      },

      links = {
        {"IAN", "github", "https://github.com/Meoo/IAN"},
        {"Help", "question-circle", "#help"},
      },

      -- Variables in this table can be accessed in client JS using "ian_cfg" table
      jscfg = {
        server_addr = "localhost",
        server_port = 7011,

        -- Random 128 bits key used to make sure the client and server match
        -- https://www.random.org/cgi-bin/randbyte?nbytes=16&format=h
        magic = "c8f6859c674e7b9d05582204bc3949d8",
      },

    }, -- shell
    
  },

}