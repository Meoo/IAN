return {

  -- CLIENT

  ["title"] = "IAN",

  ["meta"] = {
    ["description"] = "Woof cap'n!",
  },

  ["links"] = {
    {"IAN", "github", "https://github.com/Meoo/IAN"},
    {"Help", "question-circle", "#help"},
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

  -- SHARED

  -- Random 128 bits key used to make sure the client and server match
  -- https://www.random.org/cgi-bin/randbyte?nbytes=16&format=h
  ["magic"] = "c8f6859c674e7b9d05582204bc3949d8",

}