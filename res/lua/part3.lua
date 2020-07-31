
pos = PlayerPos()

for i = 0,200 do
vel = normalize(Vec2(math.random(0,100)/100,math.random(-100,100)/100))*math.random(-50,50)/50
acc = vel*-0.1
 world:spawnParticle(Particle.torch_fire,pos,vel,acc*0.1,240,40,half_int.new(-1,-1))
end