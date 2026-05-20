// 发光

void Bloom::begin()
{
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glClear(GL_COLOR_BUFFER_BIT);
}

void Bloom::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}