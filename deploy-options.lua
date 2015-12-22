return {

  -- CLIENT

  ["title"] = "IAN",

  ["meta"] = {
    ["description"] = "Woof cap'n!",
  },

  ["links"] = {
    {"IAN", "github", "https://github.com/Meoo/IAN"},
    {"Help", "question-circle", "#help"},
    {"Register", "sign-in", "#register"},
  },

  ["server-addr"] = "localhost",
  ["server-port"] = "7011",

  -- SERVER

  ["tls"] = {
    --["cert"] = path.getabsolute "localhost.pem",
    ["certchain"] = path.getabsolute "localhost.pem",
    ["key"] = path.getabsolute "localhost.pem",
    ["keypass"] = "IANkey",
    ["dh"] = path.getabsolute "dh.pem", -- openssl dhparam -out dh.pem 2048
  },

}