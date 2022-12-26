const char HTML_header[] = R"=====(
<html>
  <head>
      <title>Cellar Fan Control</title>
  </head>
<body>
    <h1>Cellar Fan Control</h1>
)=====";

const char HTML_footer[] = R"=====(
    </div>
  </div>
</body>
</html>
)=====";

const char HTML_form_header[] = R"=====(
  <form action="/save" method="POST">
)=====";

const char HTML_form_footer[] = R"=====(
    <input type="submit" value="Save">
  </form>
)=====";

const char HTML_saved_link[] = R"=====(
  <a href="/" >Information saved - click here to return to the homepage</a>
)=====";
