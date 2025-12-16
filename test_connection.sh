#!/bin/bash

# Test script to verify bottop can connect and query the database

echo "Testing SSH connection..."
ssh root@testing-azerothcore.rollet.family "echo 'SSH OK'"

echo ""
echo "Testing container status..."
ssh root@testing-azerothcore.rollet.family "docker ps --filter name=26a5e2cc361e_testing-ac-worldserver --format '{{.Status}}'"

echo ""
echo "Testing excluded accounts query..."
ssh root@testing-azerothcore.rollet.family "docker exec 26a5e2cc361e_testing-ac-worldserver mysql -htesting-ac-database -uroot -ppassword -Dacore_characters -sN -e 'SELECT GROUP_CONCAT(id) FROM acore_auth.account WHERE username IN (\"HAVOC\",\"JOSHG\",\"JOSHR\",\"JON\",\"CAITR\",\"COLTON\",\"KELSEYG\",\"KYLAN\",\"SETH\",\"AHBOT\");' 2>/dev/null"

echo ""
echo "Testing bot count query..."
ssh root@testing-azerothcore.rollet.family "docker exec 26a5e2cc361e_testing-ac-worldserver mysql -htesting-ac-database -uroot -ppassword -Dacore_characters -sN -e 'SELECT COUNT(*) FROM characters WHERE online = 1 AND account NOT IN (11160,10056,21387,1052,21388,10053,10054,10055,10057,21389);' 2>/dev/null"

echo ""
echo "Testing zones query..."
ssh root@testing-azerothcore.rollet.family "docker exec 26a5e2cc361e_testing-ac-worldserver mysql -htesting-ac-database -uroot -ppassword -Dacore_characters -sN -e 'SELECT z.name, COUNT(*) FROM characters c JOIN acore_world.zone_template z ON z.id = c.zone WHERE c.online = 1 AND c.account NOT IN (11160,10056,21387,1052,21388,10053,10054,10055,10057,21389) GROUP BY c.zone ORDER BY COUNT(*) DESC LIMIT 5;' 2>/dev/null"

echo ""
echo "All tests complete!"
