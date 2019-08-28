PREFIX = /usr

TEMPLATE  = aux

gschema.path = $${PREFIX}/share/glib-2.0/schemas
gschema.files = *.gschema.xml

INSTALLS += gschema
